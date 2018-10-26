For Installation:

1. Run nmake to build. Be sure to build with the suitable toolchain, for x86 dlls do not load under x64.
2.Copy the compiled dll "WelcomeMessager.dll" to System32 directory.
3.Import Register.reg
4.Customize the message with C:\Welcome.msg.
(Note:If you want to use a file with another path, please manually change the value HKEY_LOCAL_MACHINE\SOFTWARE\Welcome\Message)



About the format

C:\Welcome.msg is a file containing plain text.
Below are details about this file.
Each line should begin with a '@' and end with '##'
Next to the @, you are supposed to write a time filter. A time filter contains four parts separated by dots. 
And these parts are, from left to right, year, month, day of month, and day of week. You will be able to find more details if you go to see the MSDN document for SYSTEMTIME.
Each part is a single star '*', a number, or a (begin-end) segment.
And, after the time filter there comes a colon ':', and then the message.
And the program will search from the top of the file to the bottom.
For example, the following will be a legal file.
@*.10.(1-7).*:国庆快乐##
@*.12.7.*:自己过生日嘤嘤嘤##
@*.*.*.*:Hello World!##



For Uninstallation:
1.Import uninstal.reg into the registry
2.Manually remove the WelcomeMessager.dll from system32 directory.
3.Manually remove the message configuration file C:\Welcome.msg.




安装：

1.执行nmake。请注意选择与系统位数匹配的工具集。
2.将WelcomeMessager.dll复制至System32目录。
3.导入Register.reg。
4.在C:\Welcome.msg中配置显示的信息。
（注：如果您准备用其它文件进行配置， 请修改 HKEY_LOCAL_MACHINE\SOFTWARE\Welcome\Message 中的路径）



信息配置文件格式
C:\Welcome.msg 是由纯文本构成的。您可以使用notepad等任何文本编辑器进行编辑。
以下为格式描述
每一行应当以 '@' 开头，以 '##' 结尾（'表示引用）
在 @ 之后，您应当指定一个时间模式. 时间模式由被.分割开的四部分组成。 它们依次表示：年，月，日，当天是一周中的第几天。关于计数的起点，请查阅MSDN上关于SYSTEMTIME的描述。
每一部分可能是以下三种模式之一：1)一个*号；2）一个数字;3)一个用(起点-终点)表达的区间。
在时间模式之后，应当有一个:，之后是要显示的消息。
程序会从上向下进行适配。
例如以下的程序：
@*.10.(1-7).*:国庆快乐##
@*.12.7.*:自己过生日嘤嘤嘤##
@*.*.*.*:Hello World!##



卸载：
1.导入 uninstal.reg
2.手动从system32目录移除 WelcomeMessager.dll
3.手动删除您建立的信息配置文件 C:\Welcome.msg.