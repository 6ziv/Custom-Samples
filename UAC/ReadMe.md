#UAC Renderer

##这是什么？
* 一个用于替换[UAC](https://msdn.microsoft.com/en-us/library/bb756945.aspx)界面的开源工具

##如何使用？
* 在x64,x86两个文件夹下有对应位数的样例程序安装包
    * 安装前，请先安装 [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145)
* 您也可以使用它的代码 以及 一些可以从[官网](https://doc.qt.io/)获得的Qt知识 来定制您自己的UAC界面

##它是否兼容我的系统
* 在发布前，此程序在Windows 7 x86,Windows 7 x64, Windows 10 x86, Windows 10 x64下均进行了兼容性测试。
* 尽管如此，由于MS并没有将UAC的设计明文公布，此软件的编写者依然无法保证此程序适用于所有Windows 7 及以上版本。
* 更无法确保它适用于未来的Windows版本（如果有的话）
* 因此，如果在使用过程中出现问题，请联系此软件编写者。见“联系方式”一章。

##如何卸载
* 安装程序会将软件信息和卸载程序写入Windows程序列表。您只需要在Windows卸载程序的列表中寻找UAC Renderer并卸载就可以了。
* 卸载程序是位于System32目录下的UACRenderer.Uninst.exe，您也可以手动运行它来卸载此程序。

##为什么具有无效的签名？
* [Windows强制完整性检查](https://social.technet.microsoft.com/wiki/contents/articles/255.forced-integrity-signing-of-portable-executable-pe-files.aspx)需要微软认证的几家签名机构出具的EV证书。而不幸的是，它们的价格都不低。
* 事实上，该数字签名对应的证书已存放在压缩包中，但是并不建议您导入它：即使签名的根证书不受信任，在测试模式下该软件也可以正常运行。

##紧急修复
* 因为UAC出错可能导致其它程序无法获得管理员权限，一旦出错，请进入Windows修复选项修复consent.exe。
* 软件在安装过程中避免删除任何文件。原始的consent.exe仍在System32文件夹中，并被重命名为consent.exe.bak。

##安全风险
* 因为该软件并没有购买Windows内核签名，因此使用时需要打开Windows测试模式才能够正常运行。这可能引起一些[安全问题](https://docs.microsoft.com/zh-cn/windows-hardware/drivers/install/the-testsigning-boot-configuration-option)。
* 为了避免在意外情况下无法快速修理，我们的仿造UAC的更改并不需要TI权限，而是继承了System32目录的权限配置。如果认为软件足够稳定，可以手动更改权限。
* 使用CryptAPI对运行的文件的数字签名进行检验时，具体检验参数可能与MS提供的版本有所不同。

##特殊声明
* 使用前请务必在[Line Store](https://store.line.me)等处购买相关表情包使用授权！

##联系方式
* 电邮：[root@6ziv.com](mailto://root@6ziv.com)

##授权信息
* 测试程序中图片来自作品集[七濑胡桃menherachan](https://space.bilibili.com/326968804?from=search&seid=16315732763199361066)，由[ぽむ](https://www.pixiv.net/member.php?id=2302136)创作。
* 测试程序中携带的Visual C++ Redistributable for Visual Studio 2015安装包属Microsoft Corporation所有。
* 测试程序中附带的dll为Qt project的组成部分，受[LGPLv3协议](https://opensource.org/licenses/LGPL-3.0)保护。
* 安装程序中使用的部分[Privexec](https://github.com/M2Team/Privexec)代码由[M2Team](https://github.com/M2Team/)开发，受[MIT协议](https://opensource.org/licenses/MIT)保护。
* 其它部分不受任何版权保护，您可以在去除或替换了上述部分之后任意使用。
