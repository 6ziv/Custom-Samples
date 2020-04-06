#ifndef MESSAGE_H
#define MESSAGE_H
#include "DesktopNotificationManagerCompat.h"
#include <NotificationActivationCallback.h>
#include <windows.ui.notifications.h>
#include <map>
#include <random>
#include <atomic>
#include <optional>
#include <functional>
class Message
{
public:
    inline static std::optional<std::function<void(void)>> toast_done;
    static void Initialize();
    static uint32_t addmsg(const std::wstring tag,const std::wstring msg);
    static void removemsg(uint32_t ticket);
private:
    inline static Microsoft::WRL::ComPtr<IToastNotifier> notifier;
    inline static std::default_random_engine rnd;
    inline static std::map<uint32_t,std::pair<Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument>,Microsoft::WRL::ComPtr<IToastNotification> > > toasts;
};

#endif // MESSAGE_H
