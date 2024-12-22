#ifndef POWER_PROFILE_MANAGER
#define POWER_PROFILE_MANAGER

#include <giomm.h>
#include <glibmm.h>

#include <functional>

class PowerProfileManager {
public:
  PowerProfileManager();
  ~PowerProfileManager();

  void set_on_update_dispatcher(std::function<void(Glib::ustring const&)> func);
  Glib::ustring get_profile();
  void set_profile(Glib::ustring const& profile);
  std::vector<Glib::ustring> get_all();

private:
  Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
  Glib::ustring m_current_profile;
  sigc::connection m_signal_changed;
  std::function<void(Glib::ustring const&)> m_on_update;
};

#endif // !POWER_PROFILE_MANAGER
