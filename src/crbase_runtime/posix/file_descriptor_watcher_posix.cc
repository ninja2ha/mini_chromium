// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase_runtime/posix/file_descriptor_watcher_posix.h"

#include "crbase/logging/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/message_loop/message_loop_current.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/sequenced_task_runner.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/threading/sequenced_task_runner_handle.h"
#include "crbase_runtime/threading/thread_checker.h"

namespace cr {

namespace {

// MessageLoopForIO used to watch file descriptors for which callbacks are
// registered from a given thread.
ThreadLocalPointer<MessageLoopForIO>* TlsMessageLoopForIo() {
  static cr::NoDestructor<ThreadLocalPointer<MessageLoopForIO>> tls;
  return tls.get();
}

}  // namespace

FileDescriptorWatcher::Controller::~Controller() {
  CR_DCHECK(sequence_checker_.CalledOnValidSequence());

  // Delete |watcher_| on the MessageLoopForIO.
  //
  // If the MessageLoopForIO is deleted before Watcher::StartWatching() runs,
  // |watcher_| is leaked. If the MessageLoopForIO is deleted after
  // Watcher::StartWatching() runs but before the DeleteSoon task runs,
  // |watcher_| is deleted from Watcher::WillDestroyCurrentMessageLoop().
  message_loop_for_io_task_runner_->DeleteSoon(
      CR_FROM_HERE, watcher_.release());

  // Since WeakPtrs are invalidated by the destructor, RunCallback() won't be
  // invoked after this returns.
}

class FileDescriptorWatcher::Controller::Watcher
    : public MessagePumpForIO::FdWatcher,
      public MessageLoopCurrent::DestructionObserver {
 public:
  Watcher(const Watcher&) = delete;
  Watcher& operator=(const Watcher&) = delete;

  Watcher(WeakPtr<Controller> controller, MessagePumpForIO::Mode mode, int fd);
  ~Watcher() override;

  void StartWatching();

 private:
  friend class FileDescriptorWatcher;

  // MessagePumpForIO::FdWatcher:
  void OnFileCanReadWithoutBlocking(int fd) override;
  void OnFileCanWriteWithoutBlocking(int fd) override;

  // MessageLoopCurrent::DestructionObserver:
  void WillDestroyCurrentMessageLoop() override;

  // The MessageLoopForIO's watch handle (stops the watch when destroyed).
  MessagePumpForIO::FdWatchController fd_watch_controller_;

  // Runs tasks on the sequence on which this was instantiated (i.e. the
  // sequence on which the callback must run).
  const RefPtr<SequencedTaskRunner> callback_task_runner_ =
      SequencedTaskRunnerHandle::Get();

  // The Controller that created this Watcher.
  WeakPtr<Controller> controller_;

  // Whether this Watcher is notified when |fd_| becomes readable or writable
  // without blocking.
  const MessagePumpForIO::Mode mode_;

  // The watched file descriptor.
  const int fd_;

  // Except for the constructor, every method of this class must run on the same
  // MessageLoopForIO thread.
  ThreadChecker thread_checker_;

  // Whether this Watcher was registered as a DestructionObserver on the
  // MessageLoopForIO thread.
  bool registered_as_destruction_observer_ = false;
};

FileDescriptorWatcher::Controller::Watcher::Watcher(
    WeakPtr<Controller> controller,
    MessagePumpForIO::Mode mode,
    int fd)
    : fd_watch_controller_(CR_FROM_HERE),
      controller_(controller),
      mode_(mode),
      fd_(fd) {
  CR_DCHECK(callback_task_runner_);
  thread_checker_.DetachFromThread();
}

FileDescriptorWatcher::Controller::Watcher::~Watcher() {
  CR_DCHECK(thread_checker_.CalledOnValidThread());
  MessageLoopCurrentForIO::Get()->RemoveDestructionObserver(this);
}

void FileDescriptorWatcher::Controller::Watcher::StartWatching() {
  CR_DCHECK(thread_checker_.CalledOnValidThread());

  if (!MessageLoopCurrentForIO::Get()->WatchFileDescriptor(
          fd_, false, mode_, &fd_watch_controller_, this)) {
    // TODO(wez): Ideally we would [D]CHECK here, or propagate the failure back
    // to the caller, but there is no guarantee that they haven't already
    // closed |fd_| on another thread, so the best we can do is Debug-log.
    CR_DLOG(Error) << "Failed to watch fd=" << fd_;
  }

  if (!registered_as_destruction_observer_) {
    MessageLoopCurrentForIO::Get()->AddDestructionObserver(this);
    registered_as_destruction_observer_ = true;
  }
}

void FileDescriptorWatcher::Controller::Watcher::OnFileCanReadWithoutBlocking(
    int fd) {
  CR_DCHECK(fd_ == fd);
  CR_DCHECK(MessagePumpForIO::WATCH_READ == mode_);
  CR_DCHECK(thread_checker_.CalledOnValidThread());

  // Run the callback on the sequence on which the watch was initiated.
  callback_task_runner_->PostTask(
      CR_FROM_HERE, BindOnce(&Controller::RunCallback, controller_));
}

void FileDescriptorWatcher::Controller::Watcher::OnFileCanWriteWithoutBlocking(
    int fd) {
  CR_DCHECK(fd_ == fd);
  CR_DCHECK(MessagePumpForIO::WATCH_WRITE == mode_);
  CR_DCHECK(thread_checker_.CalledOnValidThread());

  // Run the callback on the sequence on which the watch was initiated.
  callback_task_runner_->PostTask(
      CR_FROM_HERE, BindOnce(&Controller::RunCallback, controller_));
}

void FileDescriptorWatcher::Controller::Watcher::
    WillDestroyCurrentMessageLoop() {
  CR_DCHECK(thread_checker_.CalledOnValidThread());

  // A Watcher is owned by a Controller. When the Controller is deleted, it
  // transfers ownership of the Watcher to a delete task posted to the
  // MessageLoopForIO. If the MessageLoopForIO is deleted before the delete task
  // runs, the following line takes care of deleting the Watcher.
  delete this;
}

FileDescriptorWatcher::Controller::Controller(MessagePumpForIO::Mode mode,
                                              int fd,
                                              const Closure& callback)
    : callback_(callback),
      message_loop_for_io_task_runner_(
          TlsMessageLoopForIo()->Get()->task_runner()),
      weak_factory_(this) {
  CR_DCHECK(!callback_.is_null());
  CR_DCHECK(message_loop_for_io_task_runner_);
  watcher_ = std::make_unique<Watcher>(weak_factory_.GetWeakPtr(), mode, fd);
  StartWatching();
}

void FileDescriptorWatcher::Controller::StartWatching() {
  CR_DCHECK(sequence_checker_.CalledOnValidSequence());
  // It is safe to use Unretained() below because |watcher_| can only be deleted
  // by a delete task posted to |message_loop_for_io_task_runner_| by this
  // Controller's destructor. Since this delete task hasn't been posted yet, it
  // can't run before the task posted below.
  message_loop_for_io_task_runner_->PostTask(
      CR_FROM_HERE, 
      BindOnce(&Watcher::StartWatching, Unretained(watcher_.get())));
}

void FileDescriptorWatcher::Controller::RunCallback() {
  CR_DCHECK(sequence_checker_.CalledOnValidSequence());

  WeakPtr<Controller> weak_this = weak_factory_.GetWeakPtr();

  callback_.Run();

  // If |this| wasn't deleted, re-enable the watch.
  if (weak_this)
    StartWatching();
}

FileDescriptorWatcher::FileDescriptorWatcher(
    MessageLoopForIO* message_loop_for_io) {
  CR_DCHECK(message_loop_for_io);
  CR_DCHECK(!TlsMessageLoopForIo()->Get());
  TlsMessageLoopForIo()->Set(message_loop_for_io);
}

FileDescriptorWatcher::~FileDescriptorWatcher() {
  TlsMessageLoopForIo()->Set(nullptr);
}

std::unique_ptr<FileDescriptorWatcher::Controller>
FileDescriptorWatcher::WatchReadable(int fd, const Closure& callback) {
  return WrapUnique(new Controller(MessagePumpForIO::WATCH_READ, fd, callback));
}

std::unique_ptr<FileDescriptorWatcher::Controller>
FileDescriptorWatcher::WatchWritable(int fd, const Closure& callback) {
  return WrapUnique(
      new Controller(MessagePumpForIO::WATCH_WRITE, fd, callback));
}

}  // namespace cr