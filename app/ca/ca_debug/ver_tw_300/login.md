// --- 发送包 ------------------------------------------------------------------------

登录账号:
CA.exe+67C8C2 - FF 35 28CCFC00        - push [CA.exe+BCCC28] { (2) }
CA.exe+67C8C8 - 8B F8                 - mov edi,eax
CA.exe+67C8CA - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67C8D0 - FF 35 24CCFC00        - push [CA.exe+BCCC24] { (2) }
CA.exe+67C8D6 - 89 BD 8CFEFFFF        - mov [ebp-00000174],edi
CA.exe+67C8DC - 6A 01                 - push 01 { 1 }                // 登陆包ID(1)
CA.exe+67C8DE - E8 7DED0C00           - call CA.exe+74B660           // 初始化包
...
CA.exe+67C905 - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67C90B - 57                    - push edi                     // edi = 账号数量
CA.exe+67C90C - E8 1FFC0C00           - call CA.exe+74C530           // WriteUint8
...
[loop, edi(账号数量)]
CA.exe+67C936 - 68 44332211           - push 11223344 { (000183F0) }
CA.exe+67C93B - 57                    - push edi                     // edi = 昵称
CA.exe+67C93C - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67C942 - E8 99FF0C00           - call CA.exe+74C8E0           // WriteStringWithEncrypt
...
CA.exe+67C9B4 - 68 11223344           - push 44332211 { (95) }
CA.exe+67C9B9 - 50                    - push eax                     // eax = 密码
CA.exe+67C9BA - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67C9C0 - E8 1BFF0C00           - call CA.exe+74C8E0           // WriteStringWithEncrypt
[loop]
...
CA.exe+67C9EA - 6A 10                 - push 10 { 16 }               // length
CA.exe+67C9EC - FF B5 84FEFFFF        - push [ebp-0000017C]          // buffer = 43259CE8 8B7FDA05 00006455 28B20000 = MAC address = GUID
CA.exe+67C9F2 - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67C9F8 - E8 D3FD0C00           - call CA.exe+74C7D0           // WriteBytes
...
CA.exe+67CA2A - 8D 8D 34FEFFFF        - lea ecx,[ebp-000001CC]
CA.exe+67CA30 - 8B 40 0C              - mov eax,[eax+0C]
CA.exe+67CA33 - 8B 00                 - mov eax,[eax]
CA.exe+67CA35 - FF 30                 - push [eax]                   // [esp] = local ip address = 5F01A8C0(little endian) = 192.168.1.95
CA.exe+67CA37 - E8 34FC0C00           - call CA.exe+74C670           // WriteUint32
...
CA.exe+67CA3C - 8D 85 34FEFFFF        - lea eax,[ebp-000001CC]
CA.exe+67CA42 - 8B CB                 - mov ecx,ebx
CA.exe+67CA44 - 50                    - push eax
CA.exe+67CA45 - E8 62FDFFFF           - call CA.exe+67C7AC           // SendPakcet

// --- 收取包 ------------------------------------------------------------------------

/*
** 获取配置文件 "fxdynimginfo.ssd" 登录成功失败都有 (ebx + 0x8 = 0xd = 13)
*/
CA.exe+37C220 - 8B 0D E0A0FF00        - mov ecx,[CA.exe+BFA0E0] { (01C4ECB0) }
CA.exe+37C226 - 53                    - push ebx                  type = ebx + 8] = 0xD
CA.exe+37C227 - E8 9EB1FBFF           - call CA.exe+3373CA

/*
** Unknow - 登录成功失败都有(ebx + 0x8 = 0x2B5= 693)
*/
CA.exe+37BDFD - 53                    - push ebx                  type = ebx + 8] = 0x2B5
CA.exe+37BDFE - 8B CF                 - mov ecx,edi
CA.exe+37BE00 - E8 2B940000           - call CA.exe+385230


/*
**  密码错误返回的包 (type = esi + 0x8] = 0x30 = 48)
*/

00DE40A0 00000000 00000030 FFFFFFFF 
00000017 00000002 00000003 00000002 
00000000 00000001 00DE52A0 00000320 
00000040 0D939770 00000000 00000000 
00000000 00000017 00000000 00000002 
00000003 00000000 0000030E 00000000
CA.exe+538858 - 56                    - push esi
CA.exe+538859 - 8B CF                 - mov ecx,edi
CA.exe+53885B - E8 88170000           - call CA.exe+539FE8
...
CA.exe+53A010 - 8B 07                 - mov eax,[edi]
CA.exe+53A012 - 8B CF                 - mov ecx,edi
CA.exe+53A014 - FF 50 3C              - call dword ptr [eax+3C] // uint8 (unknow)
CA.exe+53A017 - 8B 07                 - mov eax,[edi]
CA.exe+53A019 - 8D 4D D8              - lea ecx,[ebp-28]
CA.exe+53A01C - 51                    - push ecx
CA.exe+53A01D - 8B CF                 - mov ecx,edi
CA.exe+53A01F - FF 50 4C              - call dword ptr [eax+4C] // string (error message)

/*
** 登录成功反馈包 (type = esi + 0x8] = 0x4)
*/

00DE40A0 00000000 00000004 FFFFFFFF 
000002BD 00000002 00000003 00000002 
00000000 00000001 00DE52A0 00000320 
000002BD 55528D28 00000000 00000000 
00000000 000002BD 00000000 00000002 
00000003 00000000 0000134A 00000000
CA.exe+53880A - 56                    - push esi
CA.exe+53880B - 8B CF                 - mov ecx,edi
CA.exe+53880D - E8 390C0000           - call CA.exe+53944B
...
CA.exe+53949C - 8B 06                 - mov eax,[esi]
CA.exe+53949E - 8B CE                 - mov ecx,esi
CA.exe+5394A0 - FF 50 3C              - call dword ptr [eax+3C] // uint8 (error code) - 登录错误码
CA.exe+5394A3 - 0FB6 C0               - movzx eax,al

登录错误码:
0x00(009394B6) = 登录成功
0x01(009395C5) = (ErrLoginNotRegistered)
0x02(009394C3) = (ErrLoginAlready)
0x03(0093979B) = (ErrLoginUnknown)
0x04(0093968A) = (ErrLoginConnectionForbidden) - 账号被锁定
0x05(0093980D) = () - 弹出注册账号框.. 没被使用
0x06(00939D52) =
0x07(00939D52) =
0x08(00939D52) =
0x09(009398AA) = (NotAllowedCountry)
0x0A(00939637) = (ErrLoginInvalidNexonId)
0x0B(0093965F) = (ErrLoginCheckNexonId)
0x0C(00939D52) =
0x0D(009398D5) = () - 自定义公告.
0x0E(00939945) = (ErrDisconnectByInvalidNewMemberShip) - 用户点否会退出游戏
0x0F(00939A8F) = () - 自定义公告
0x10(00939AB8) = (OneClick_FailAcqureOneClickId)
0x11(00939AE3) = ()
0x12(00939D52) =
0x13(00939D52) =
0x14(00939C2D) = (ErrLoginFailShutDownByPlayTime)
0x15(00939C58) = (ErrLoginFailShutDownByFCM)
0x16(00939C83) = (LoginRetWithdrawalSimpleGameUser)
0x17(00939CAE) = (ErrLoginSDOBase)

; [登录错误码]: 0 - 解析
CA.exe+5394B6 - 56                    - push esi
CA.exe+5394B7 - 8B CF                 - mov ecx,edi
CA.exe+5394B9 - E8 F6F6FFFF           - call CA.exe+538BB4
...
CA.exe+538BD4 - 8B 03                 - mov eax,[ebx]
CA.exe+538BD6 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0x0)
CA.exe+538BD9 - 0FB6 C0               - movzx eax,al
...
CA.exe+538BEE - FF 50 40              - call dword ptr [eax+40]  uint16(0x1)
CA.exe+538BF1 - 0FB7 C0               - movzx eax,ax
...
CA.exe+538C0A - FF 50 3C              - call dword ptr [eax+3C]  uint8(1) - 登录账号数量(1P || 2P)
CA.exe+538C0D - 0FB6 F8               - movzx edi,al
...
CA.exe+538C1A - FF 50 48              - call dword ptr [eax+48]  uint64 (0x000000006A323D90) - Tickount64
CA.exe+538C1D - 52                    - push edx
CA.exe+538C1E - 50                    - push eax
..
.. [LOOP]=>登录账号数量 - begin
...
CA.exe+538D80 - 8B 35 7C95FF00        - mov esi,[CA.exe+BF957C]
CA.exe+538D86 - FF 50 44              - call dword ptr [eax+44]  uint32 (0)
CA.exe+538D89 - 50                    - push eax
..
CA.exe+538D94 - 8B CB                 - mov ecx,ebx
CA.exe+538D96 - FF 50 44              - call dword ptr [eax+44]  uint32 (0)
CA.exe+538D99 - 8B 13                 - mov edx,[ebx]
...
CA.exe+538D9D - 8B F8                 - mov edi,eax
CA.exe+538D9F - FF 52 40              - call dword ptr [edx+40]  uint16 (0)
CA.exe+538DA2 - 8B 13                 - mov edx,[ebx]
...
CA.exe+538DA6 - 0FB7 F0               - movzx esi,ax
CA.exe+538DA9 - FF 52 40              - call dword ptr [edx+40]  uint16 (0)
CA.exe+538DAC - 0FB7 C8               - movzx ecx,ax
...
CA.exe+538DC5 - 8B CB                 - mov ecx,ebx
CA.exe+538DC7 - FF 50 44              - call dword ptr [eax+44]  uint32 (0x31435) 登录序号
CA.exe+538DCA - 8B 13                 - mov edx,[ebx]
..
CA.exe+538DD3 - 50                    - push eax
CA.exe+538DD4 - FF 52 4C              - call dword ptr [edx+4C]  string (用户昵称)
CA.exe+538DD7 - C6 45 FC 04           - mov byte ptr [ebp-04],04
...
CA.exe+538DDD - 8B 13                 - mov edx,[ebx]
CA.exe+538DDF - FF 52 3C              - call dword ptr [edx+3C]  uint8 (0)
CA.exe+538DE2 - 8B 0D B495FF00        - mov ecx,[CA.exe+BF95B4]
...
CA.exe+538E2E - 51                    - push ecx
CA.exe+538E2F - 8B CB                 - mov ecx,ebx
CA.exe+538E31 - FF 50 4C              - call dword ptr [eax+4C]  string
CA.exe+538E34 - C6 45 FC 05           - mov byte ptr [ebp-04],05
...
CA.exe+538E59 - 8D 8D 60FFFFFF        - lea ecx,[ebp-000000A0]
CA.exe+538E5F - 51                    - push ecx
CA.exe+538E60 - 8B CB                 - mov ecx,ebx
CA.exe+538E62 - FF 50 4C              - call dword ptr [eax+4C]  string
CA.exe+538E65 - C6 45 FC 06           - mov byte ptr [ebp-04],06 
...
CA.exe+538E8A - 8B CB                 - mov ecx,ebx
CA.exe+538E8C - 8B 35 B495FF00        - mov esi,[CA.exe+BF95B4] 
CA.exe+538E92 - FF 50 3C              - call dword ptr [eax+3C]  uint8 (0)
CA.exe+538E95 - 0FB6 C0               - movzx eax,al
...
CA.exe+538EA5 - 8B 35 B495FF00        - mov esi,[CA.exe+BF95B4] 
CA.exe+538EAB - FF 50 48              - call dword ptr [eax+48]  uint64 (0)
CA.exe+538EAE - 52                    - push edx
...
CA.exe+538EC2 - FF 50 44              - call dword ptr [eax+44]  uint32 (0)
CA.exe+538EC5 - 50                    - push eax
..
CA.exe+538ED8 - FF 50 3C              - call dword ptr [eax+3C]  uint8 (0)
CA.exe+538EDB - 0FB6 C0               - movzx eax,al
...
CA.exe+538EF1 - FF 50 40              - call dword ptr [eax+40]  uint16(0x4A = 74) - 等级
CA.exe+538EF4 - 0FB7 C0               - movzx eax,ax
..
CA.exe+538F0A - FF 50 3C              - call dword ptr [eax+3C]  uint8(0xA = 10) - 段位
CA.exe+538F0D - 0FB6 C0               - movzx eax,al
..
CA.exe+538F23 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+538F26 - 0FB6 C0               - movzx eax,al
..
CA.exe+538F3C - FF 50 44              - call dword ptr [eax+44]  uint32(0)
CA.exe+538F3F - 50                    - push eax
..
CA.exe+538F52 - FF 50 44              - call dword ptr [eax+44]  uint32(0)
CA.exe+538F55 - 50                    - push eax
..
CA.exe+538F68 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+538F6B - 0FB6 C0               - movzx eax,al
..
CA.exe+538F81 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+538F84 - 0FB6 C0               - movzx eax,al
...
CA.exe+538F9A - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+538F9D - 0FB6 C0               - movzx eax,al
...
..cmp push 6
...
CA.exe+538FBF - FF 50 40              - call dword ptr [eax+40]  uint16(2) - 家族加入状态
CA.exe+538FC2 - 0FB7 C0               - movzx eax,ax
...
CA.exe+538FD8 - FF 50 40              - call dword ptr [eax+40]  uint16(3) - 家族身份
CA.exe+538FDB - 0FB7 C0               - movzx eax,ax
...
CA.exe+538FF1 - FF 50 44              - call dword ptr [eax+44]  uint32(0x0033E58) - 家族ID
CA.exe+538FF4 - 50                    - push eax
...
CA.exe+53902D - FF 50 3C              - call dword ptr [eax+3C]  uint8(2) - unknow value
CA.exe+539030 - 0FB6 C0               - movzx eax,al
...
CA.exe+539046 - FF 50 3C              - call dword ptr [eax+3C]  uint8(2) - unknow value
CA.exe+539049 - 0FB6 C0               - movzx eax,al
...
CA.exe+53905F - FF 50 44              - call dword ptr [eax+44]  uint32(0)
CA.exe+539062 - 50                    - push eax
...
CA.exe+539075 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+539078 - 0FB6 C0               - movzx eax,al
...
CA.exe+539088 - FF 50 3C              - call dword ptr [eax+3C]  uint8(5)
CA.exe+53908B - 8B 0D B495FF00        - mov ecx,[CA.exe+BF95B4]
...
CA.exe+5390BE - FF 50 44              - call dword ptr [eax+44]  uint32(1)
CA.exe+5390C1 - 8B F0                 - mov esi,eax
...
...cmp 1
...
CA.exe+5390CB - FF 52 44              - call dword ptr [edx+44]  uint32(5)
CA.exe+5390CE - 89 85 44FFFFFF        - mov [ebp-000000BC],eax
...
CA.exe+5390D8 - FF 50 44              - call dword ptr [eax+44]  uint32(0xA)
CA.exe+5390DB - 89 85 48FFFFFF        - mov [ebp-000000B8],eax
...
... cmp 1
..
CA.exe+539115 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+539118 - 0FB6 C0               - movzx eax,al
...
... [LOOP] => 登录账号数量 - end
...
CA.exe+539226 - FF 50 3C              - call dword ptr [eax+3C]  uint8(1)
CA.exe+539229 - 0FB6 C0               - movzx eax,al
...
CA.exe+53923E - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+539241 - 0FB6 C0               - movzx eax,al
...
CA.exe+539256 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+539259 - 0FB6 C0               - movzx eax,al
..
CA.exe+53926E - FF 50 40              - call dword ptr [eax+40]  uint16(0x3A98) 15000(15秒) ? 计时用的
CA.exe+539271 - 0FB7 C0               - movzx eax,ax
..
CA.exe+539286 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0)
CA.exe+539289 - 0FB6 C0               - movzx eax,al
..
CA.exe+539298 - FF 50 3C              - call dword ptr [eax+3C]  uint8(0) - NeedToChangePassword
CA.exe+53929B - 84 C0                 - test al,al
..
CA.exe+5393DF - 53                    - push ebx
CA.exe+5393E0 - E8 71F80E00           - call CA.exe+628C56       - 解析段位信息
..
.. 解析段位信息 - begin
..
CA.exe+66B63B - FF 50 44              - call dword ptr [eax+44]  uint32(0x1C) - 段位赛季数量
CA.exe+66B63E - 33 C9                 - xor ecx,ecx
..
CA.exe+66B67C - FF 50 44              - call dword ptr [eax+44]  uint32(0x1A) - 段位赛季ID
CA.exe+66B67F - 89 45 CC              - mov [ebp-34],eax
..
CA.exe+66B686 - FF 50 40              - call dword ptr [eax+40]  uint16(0x7EA) - 段位赛季年份
CA.exe+66B689 - 66 89 45 D0           - mov [ebp-30],ax
...
CA.exe+66B691 - FF 50 3C              - call dword ptr [eax+3C]  uint8(1) - 段位赛季当年ID (春=1, 夏=2, 秋=3, 冬=4)
CA.exe+66B694 - 88 45 D2              - mov [ebp-2E],al
..
CA.exe+66B69F - FF 50 4C              - call dword ptr [eax+4C]  string(段位赛季昵称)
CA.exe+66B6A2 - C6 45 FC 04           - mov byte ptr [ebp-04],04
...
CA.exe+66B6C3 - FF 50 3C              - call dword ptr [eax+3C]  uint8(1)
CA.exe+66B6C6 - 83 7D B0 00           - cmp dword ptr [ebp-50],00 
...
.. 解析段位信息 - end
..

家族加入状态:
0x00 = ；没有公会
0x02 = ；有加入的公会
0x03 = (UserBannedGuild) - 您已被组长强制退出家族。
0x04 = (GuildJoinSuccess2) - 家族加入完成
0x05 = (GuildWaitEndFail) - 申请加入家族被拒绝
0x06 = (GuildReq7dayExpired) - 您提出的家族申请已经超过7天，但还没收到任何回复!系统已经取消您的申请记录。
0x64 = (GuildAcceptedButLimitOver) - 加入家族申请被接受，但是家族人数已经满了。

家族身份:
0x00 = 正式成员
0x01 = 高手护法
0x01 = 家族高层
0x03 = 族长

-------------------------------------------------------------------------------------------

/*
** [0093E334]服务器选择大厅接受的包(type = 0x33 = 51) 个人道具信息?
*/
CA.exe+37BFD8 - 53                    - push ebx
CA.exe+37BFD9 - 8B CF                 - mov ecx,edi
CA.exe+37BFDB - E8 A6260000           - call CA.exe+37E686
