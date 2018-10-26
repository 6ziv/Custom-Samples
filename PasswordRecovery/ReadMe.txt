#Passrec
Record the login passwords on this computer (which are passed to msv1_0)
Installation.exe is not supposed to be used under Windows : it is designed to be used under the cmd within windows startup repairation.

For installing within windows environment, add "PassRec" to "HKEY_LOCAL_MACHINE\CurrentControlSet\Control\Lsa\Authentication Packages"
And then copy PassRec.dll to System32 directory.

If you do not want it installed on your computer, please carefully configure privileges to the registry and System32 directory.

#密码记录者
记录登录时传递给msv1_0的密码。
为了避免滥用，提供的自动安装工具Installation.exe并不能在Windows下运行：它应当在cmd启动修复模式下提供的命令提示符中运行。

如果要在Windows环境下安装，您需要手动在HKEY_LOCAL_MACHINE\CurrentControlSet\Control\Lsa\Authentication Packages键值下添加字符串"PassRec"，并将 PassRec.dll 复制到 System32 目录。

如果您不希望它，或者此类软件被安装在您的计算机上，请配制注册表与System32目录的权限。
