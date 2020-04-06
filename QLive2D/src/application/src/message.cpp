#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"runtimeobject.lib")
#include "message.h"
#include "qdebug.h"
#include "pluginloader.hpp"
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;

class DECLSPEC_UUID("3a3b3c1f-2c81-469a-baf7-09d6bbe5cd1b") NotificationActivator WrlSealed WrlFinal
    : public RuntimeClass<RuntimeClassFlags<ClassicCom>, INotificationActivationCallback>
{
public:
    virtual HRESULT STDMETHODCALLTYPE Activate(
        _In_ LPCWSTR appUserModelId,
        _In_ LPCWSTR invokedArgs,
        _In_reads_(dataCount) const NOTIFICATION_USER_INPUT_DATA* data,
        ULONG dataCount) override
    {
        const wchar_t *ptr=invokedArgs;
        while((*ptr!=L'\0') && (*ptr!=L'?'))ptr++;
        if(*ptr==L'\0'){
            std::invoke(Message::toast_done.value());
            return S_OK;
        }
        std::wstring dll=std::wstring(invokedArgs,ptr-invokedArgs);
        std::wstring flag=std::wstring(ptr+1);
        PluginLoader::activate(dll,flag);
        std::invoke(Message::toast_done.value());
        return S_OK;
    }
};
CoCreatableClass(NotificationActivator);

void Message::Initialize()
{
    DesktopNotificationManagerCompat::RegisterAumidAndComServer(L"SixZiv.QtDesktopLive2d",__uuidof(NotificationActivator));
    DesktopNotificationManagerCompat::RegisterActivator();
if(toast_done)return;
    DesktopNotificationManagerCompat::CreateToastNotifier(&notifier);
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    rnd.seed(timestamp.QuadPart);
}
uint32_t Message::addmsg(const std::wstring tag,const std::wstring msg){
    ComPtr<IXmlDocument> doc;
    const std::wstring head1=L"<toast launch=\"";
    const std::wstring head2=L"\"><visual><binding template='ToastGeneric'><text>";
    const std::wstring tail=L"</text></binding></visual></toast>";
    std::wstring xml=head1+tag+head2+msg+tail;
    HRESULT hr=DesktopNotificationManagerCompat::CreateXmlDocumentFromString(xml.c_str(),&doc);
    if (!SUCCEEDED(hr))return 0;
    ComPtr<IToastNotification> toast;

    hr = DesktopNotificationManagerCompat::CreateToastNotification(doc.Get(), &toast);
    if (!SUCCEEDED(hr))return 0;
    hr=notifier->Show(toast.Get());
    if (!SUCCEEDED(hr))return 0;

    uint32_t id=0;
    do{
        id=rnd();
    }while((id==0) || (toasts.find(id)!=toasts.cend()));
    toasts[id]=std::make_pair(doc,toast);

    return id;
}
void Message::removemsg(uint32_t ticket){
    qDebug()<<"removing";
    if(toasts.find(ticket)==toasts.cend())return;
    qDebug()<<"removal";
    notifier->Hide(toasts[ticket].second.Get());
    toasts.erase(ticket);
}
