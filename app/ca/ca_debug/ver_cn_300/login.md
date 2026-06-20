; 登录成功反馈包(packet type = 4)

CA.exe+CD33AF - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CD33B2 - FF D0                 - call eax     ; uint8 - 登录错误码(0 = 成功)
CA.exe+CD33B4 - 0FB6 C8               - movzx ecx,al
...
...[登录错误码 = 0] - begin
...
CA.exe+CDC425 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDC428 - FF D0                 - call eax    ; uint8 (0)
CA.exe+CDC42A - 0FB6 C8               - movzx ecx,al
...
CA.exe+CDC458 - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDC45B - FF D0                 - call eax    ; uint16 (1)
CA.exe+CDC45D - 66 89 85 96F4FFFF     - mov [ebp-00000B6A],ax
...
CA.exe+CDC497 - 52                    - push edx
CA.exe+CDC498 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC49B - FF 95 F0F2FFFF        - call dword ptr [ebp-00000D10] ; string ()
CA.exe+CDC4A1 - 89 85 ECF2FFFF        - mov [ebp-00000D14],eax
...
CA.exe+CDC4FD - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDC500 - FF D0                 - call eax     ;  uint8 (1) - 登录账号数量(1P | 2P)
CA.exe+CDC502 - 0FB6 C8               - movzx ecx,al
CA.exe+CDC505 - 89 8D 9CF4FFFF        - mov [ebp-00000B64],ecx
...
... [账号信息LOOP][ebp-00000B64] - begin
...
CA.exe+CDC669 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC66C - 8B 50 44              - mov edx,[eax+44]
CA.exe+CDC66F - FF D2                 - call edx      ; uint32 (0)
CA.exe+CDC671 - 89 85 7CF4FFFF        - mov [ebp-00000B84],eax
...
CA.exe+CDC699 - 8B 50 44              - mov edx,[eax+44]
CA.exe+CDC69C - FF D2                 - call edx      ; uint32(22F83,2965E) login_seq; 登录序号
CA.exe+CDC69E - 89 85 70F4FFFF        - mov [ebp-00000B90],eax
...
CA.exe+CDC6B9 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC6BC - FF 95 74F4FFFF        - call dword ptr [ebp-00000B8C] ; string (FBC3BBD2 E3D0C5D3 D1D3F8CD 00000000) - 账号昵称
CA.exe+CDC6C2 - C7 45 FC 02000000     - mov [ebp-04],00000002 { 2 }
...
CA.exe+CDC732 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC735 - FF 95 64F4FFFF        - call dword ptr [ebp-00000B9C] ; string (en00735568760.pt) - 通行证ID
CA.exe+CDC73B - 89 85 60F4FFFF        - mov [ebp-00000BA0],eax
...
CA.exe+CDC7A6 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC7A9 - FF 95 50F4FFFF        - call dword ptr [ebp-00000BB0] ; string (PTID)
CA.exe+CDC7AF - 89 85 4CF4FFFF        - mov [ebp-00000BB4],eax
...
CA.exe+CDC81A - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC81D - FF 95 3CF4FFFF        - call dword ptr [ebp-00000BC4] ; string (PTID)
CA.exe+CDC823 - 89 85 38F4FFFF        - mov [ebp-00000BC8],eax
...
CA.exe+CDC881 - 8B 50 3C              - mov edx,[eax+3C]
CA.exe+CDC884 - FF D2                 - call edx　　　　　　　 ; uint8 (0)
CA.exe+CDC886 - 0FB6 C0               - movzx eax,al
...
CA.exe+CDC8FD - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC900 - FF 95 1CF4FFFF        - call dword ptr [ebp-00000BE4] ; string ()
CA.exe+CDC906 - 89 85 18F4FFFF        - mov [ebp-00000BE8],eax
...
CA.exe+CDC976 - 50                    - push eax
CA.exe+CDC977 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDC97A - FF 95 08F4FFFF        - call dword ptr [ebp-00000BF8] ; string ()
CA.exe+CDC980 - 89 85 04F4FFFF        - mov [ebp-00000BFC],eax
...
CA.exe+CDC9E4 - 8B 50 3C              - mov edx,[eax+3C]
CA.exe+CDC9E7 - FF D2                 - call edx  ; uint8 (0)
CA.exe+CDC9E9 - 0FB6 C0               - movzx eax,al
...
CA.exe+CDCA1E - 8B 42 44              - mov eax,[edx+44]
CA.exe+CDCA21 - FF D0                 - call eax  ; uint32 (000469EE) 469E3 过期商品??
CA.exe+CDCA23 - 89 85 ECF3FFFF        - mov [ebp-00000C14],eax
...
CA.exe+CDCA55 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCA58 - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCA5A - 0FB6 C8               - movzx ecx,al
...
CA.exe+CDCA8F - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDCA92 - FF D0                 - call eax ; uint16 (D2) - 等级
CA.exe+CDCA94 - 66 89 85 98F4FFFF     - mov [ebp-00000B68],ax
...
CA.exe+CDCAC8 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCACB - FF D0                 - call eax  ; uint8 (0) - 段位
CA.exe+CDCACD - 88 85 ADF4FFFF        - mov [ebp-00000B53],al
...
CA.exe+CDCB00 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCB03 - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCB05 - 88 85 ACF4FFFF        - mov [ebp-00000B54],al
...
CA.exe+CDCB38 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCB3B - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCB3D - 88 85 ABF4FFFF        - mov [ebp-00000B55],al
...
CA.exe+CDCB70 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCB73 - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCB75 - 88 85 AAF4FFFF        - mov [ebp-00000B56],al
...
CA.exe+CDCBA8 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCBAB - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCBAD - 88 85 AFF4FFFF        - mov [ebp-00000B51],al
... cmp push 06
CA.exe+CDCBF3 - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDCBF6 - FF D0                 - call eax ; uint16 (0,2) ； 家族加入状态
CA.exe+CDCBF8 - 0FB7 C8               - movzx ecx,ax
...
CA.exe+CDCC2D - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDCC30 - FF D0                 - call eax ; uint16 (0)   ； 家族身份
CA.exe+CDCC32 - 0FB7 C8               - movzx ecx,ax
... cmp
CA.exe+CDCCB5 - 8B 42 44              - mov eax,[edx+44]
CA.exe+CDCCB8 - FF D0                 - call eax ; uint32 (0) 0005789A(家族ID??)
CA.exe+CDCCBA - 89 85 ACF3FFFF        - mov [ebp-00000C54],eax
...
CA.exe+CDCD0D - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCD10 - FF D0                 - call eax ; uint8 (0)
CA.exe+CDCD12 - 88 85 AEF4FFFF        - mov [ebp-00000B52],al
...
CA.exe+CDCD50 - 8B 50 44              - mov edx,[eax+44]
CA.exe+CDCD53 - FF D2                 - call edx ; uint32 (0)  - count
CA.exe+CDCD55 - 89 85 9CF3FFFF        - mov [ebp-00000C64],eax 
...
...[Loop][ebp-00000C64] - begin
...
CA.exe+CDCD97 - 8B 50 44              - mov edx,[eax+44]
CA.exe+CDCD9A - FF D2                 - call edx ; uint32
CA.exe+CDCD9C - 89 85 FCF2FFFF        - mov [ebp-00000D04],eax
CA.exe+CDCDAA - 8B 42 44              - mov eax,[edx+44]
CA.exe+CDCDAD - FF D0                 - call eax  ; uint32
CA.exe+CDCDAF - 89 85 00F3FFFF        - mov [ebp-00000D00],eax
...
...[Loop][ebp-00000C64] - end
...
CA.exe+CDCE01 - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDCE04 - FF D0                 - call eax  ; uint16 (36E)
CA.exe+CDCE06 - 0FB7 C8               - movzx ecx,ax
...
CA.exe+CDCF4F - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDCF52 - FF D0                 - call eax  ; uint8 (0)
CA.exe+CDCF54 - 0FB6 C8               - movzx ecx,al
...
CA.exe+CDCFBA - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+CDCFBD - FF 95 70F3FFFF        - call dword ptr [ebp-00000C90] ; string (89679c3392bc187eca92b54327c39256) - session ?
CA.exe+CDCFC3 - C6 45 FC 0B           - mov byte ptr [ebp-04],0B { 11 }
...
... [账号信息LOOP][ebp-00000B64] - end
...
CA.exe+CDD0CB - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDD0CE - FF D0                 - call eax ; uint8 (1)
CA.exe+CDD0D0 - 0FB6 C8               - movzx ecx,al
...
CA.exe+CDD107 - 8B 45 08              - mov eax,[ebp+08]
CA.exe+CDD10A - 50                    - push eax
CA.exe+CDD10B - 8B 8D 50F3FFFF        - mov ecx,[ebp-00000CB0]
CA.exe+CDD111 - E8 5A652100           - call CA.exe+EF3670 ; 读取UI配置信息(段位赛季信息...)
...
... UI配置信息 - begin
...
CA.exe+F898A4 - 8B 42 44              - mov eax,[edx+44]
CA.exe+F898A7 - FF D0                 - call eax ; uint32 (0x49) - 自由段位赛季信息数量
CA.exe+F898A9 - 8B 4D A4              - mov ecx,[ebp-5C]
...
...[LOOP] - begin
CA.exe+F898FD - 8B 50 44              - mov edx,[eax+44]
CA.exe+F89900 - FF D2                 - call edx ; uint32 (0x48) - 赛季ID 2026 春(0x48) 夏(0x49) 2025 春(0x44) 夏(0x45) 秋(0x46) 冬(0x47)
CA.exe+F89902 - 89 45 C0              - mov [ebp-40],eax
...
CA.exe+F8990D - 8B 42 40              - mov eax,[edx+40]
CA.exe+F89910 - FF D0                 - call eax ; uint16 (0x7EA) - 赛季年份
CA.exe+F89912 - 66 89 45 C4           - mov [ebp-3C],ax
...
CA.exe+F8991E - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+F89921 - FF D0                 - call eax ; uint8 (1) - 赛季当年ID (春=1, 夏=2, 秋=3, 冬=4)
CA.exe+F89923 - 88 45 C6              - mov [ebp-3A],al
...
CA.exe+F89935 - 8B 4D 08              - mov ecx,[ebp+08]
CA.exe+F89938 - FF 55 88              - call dword ptr [ebp-78] ; string () - 赛季昵称(国服使用GBK编码)
CA.exe+F8993B - 89 45 84              - mov [ebp-7C],eax
...
CA.exe+F89969 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+F8996C - FF D0                 - call eax  ; uint8 (2) - always as 2 -不知道是啥，不影响游戏内容
CA.exe+F8996E - 88 45 E0              - mov [ebp-20],al
... - end
...
CA.exe+F89A41 - 8B 50 44              - mov edx,[eax+44]
CA.exe+F89A44 - FF D2                 - call edx  ; - 其他信息数量(0)
CA.exe+F89A46 - 89 85 74FFFFFF        - mov [ebp-0000008C],eax
..
.. UI配置信息 - end
..
CA.exe+CDD163 - 8B 42 3C              - mov eax,[edx+3C]
CA.exe+CDD166 - FF D0                 - call eax ; uint8 (0)
CA.exe+CDD168 - 0FB6 C8               - movzx ecx,al
...
CA.exe+CDD196 - 8B 42 40              - mov eax,[edx+40]
CA.exe+CDD199 - FF D0                 - call eax  ; uint16 (EA60) - 60000ms 计时??
CA.exe+CDD19B - 66 89 85 9AF4FFFF     - mov [ebp-00000B66],ax
...
CA.exe+CDD1BD - 8B 50 3C              - mov edx,[eax+3C]
CA.exe+CDD1C0 - FF D2                 - call edx  ; uint8 (1) - ContractImage(0 = enable, 1=diable)?? 展示同意协议
CA.exe+CDD1C2 - 0FB6 C0               - movzx eax,al
...
CA.exe+CDD2DD - 8B 50 3C              - mov edx,[eax+3C]
CA.exe+CDD2E0 - FF D2                 - call edx  ; uint8 (0)
CA.exe+CDD2E2 - 0FB6 C0               - movzx eax,al
...
CA.exe+CDD2F9 - 8D 4D A8              - lea ecx,[ebp-58]
CA.exe+CDD2FC - 51                    - push ecx
CA.exe+CDD2FD - 8B 4D 08              - mov ecx,[ebp+08] ; string ()
CA.exe+CDD300 - FF 95 2CF3FFFF        - call dword ptr [ebp-00000CD4]
...
...[登录错误码 = 0] - end
...

登录错误码:
0x00 = 登录成功
0x01 = (ErrLoginNotRegistered) - 盛趣通行证或密码有误，请您确认后重新输入
0x02 = (ErrLoginAlready) - 该账号已经在线，需要断开连接重新登录吗？
0x03 = (ErrLoginUnknown) - 因发生了无故的错误，连接失败
0x04 = (ErrLoginConnectionForbidden) - 由于您当前账号（角色）违反法律法规或用户协议，故停止服务。  如有异议请联系客服热线95105222
0x05 = () - [弹出新的账号登录框, 注册用]
0x07 = (ErrWrongReservedPwd) - 输入错误的服务号
0x08 = (ErrLoginDigitId) - 无法使用数字账号登录
0x09 = (NotAllowedCountry) - 
0x10 = (OneClick_FailAcqureOneClickId) - 登录失败，没有剩余的体验ID。
0x12 = (ErrDynamicPassword) - 动态密码错误（静态密码功能已被关闭）
0x13 = (SDO_WrongAuthInfo) - 
0x16 = (ErrSDOBaseAuthFail)
0x17 = (ErrLoginFailChinaShutDownNight) - 当前非未成年人游戏时间，请您于每周五至周日（含法定节假日）20-21点进行登录。
0x18 = (ErrLoginFailChinaShutDownTime) - 您今日的游戏时长已耗尽。
0x19 = (ErrLoginFailChinaShutDownAge) - 18岁以下玩家无法登录游戏
0x1A = (ErrLoginFailChinaShutDownAuthentication) - 根据....配合进行实名注册。
0x1B = (ErrLoginFailChinaShutDownReAuthRealName) - 您的实名注册信息的验证结果为：验证未通过...请放心填写。
0x1C = (ErrLoginFailPhoneLinkLimit) - 登录游戏前，请访问i.sdo.com进行认证手机绑定。
0x1D = (ErrLoginFailIpLimitCount) - 今日此 IP 登录账号数量已达上线，次日零点自动重置，请明日再尝试登录。
0x1E = (ErrLogin2PSameId) - 登录相同的账号，无法进行2P登录。

CA.exe+3E387F - E8 6C29BD00           - call CA.exe+Fren'sB61F0
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
