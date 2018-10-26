To install, Run the following commands with administrator privilege by order (under src directory).
安装请以管理员特权依次执行以下命令(在src目录下)。

nmake
nmake -install
install

Then, turn on bluetooth on both your computer and your phone, set your phone as discoverable, and start the configuration tool. Use the up/down arrows and Page up/down to select your phone, and press Enter to confirm. During configuration, you can press Esc to quit at and time, but you will need to finish the configuration later. If you can press F5 to re-discover the remote devices. 
然后打开计算机和远程设备的蓝牙开关，并配置远程设备为可发现。之后进入配置工具。用上下键/PageUp/PageDown选择您的设备。按下Enter以确认。在过程中，您可以按下ESC以退出，并在之后再进行配置。如果您的设备没有被发现，您也可以按下F5以重新检索。

Note that this project is just for fun and it provides no security. And, when bluetooth is turned off, the default action is to let any login attempts pass. Therefore, is you have any trouble logging in, you should switch your computer to flight mode, and then login.
注：本工具仅供娱乐。在蓝牙关闭的前提下，一切登录将被默认放行。如果出现无法开机的情况，请通过登录页面右下角的网络配置将计算机调节至飞行模式，再进行登录。

To uninstall, run bin/uninstall.reg.
如需卸载，请运行bin文件夹中的uninstall.reg。

And you may need to manually remove SubAuth.dll under system32 directory.
如果要彻底删除…您需要在卸载后删除system32目录下的SubAuth.dll