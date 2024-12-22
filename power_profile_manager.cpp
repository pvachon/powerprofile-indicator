#include "power_profile_manager.h"

#include <functional>
#include <iostream>

#define assert_ambiguous_output(condition, data, ret)                          \
  if (condition) {                                                             \
    std::cerr << "Dbus output is ambiguous!" << std::endl;                     \
    std::cerr << data.print();                                                 \
    return ret;                                                                \
  }

PowerProfileManager::PowerProfileManager()
    : m_on_update([](Glib::ustring const&) {})
{

  Gio::init();
  Glib::RefPtr<Gio::DBus::Connection> connection =
      Gio::DBus::Connection::get_sync(Gio::DBus::BusType::BUS_TYPE_SYSTEM);

  if (!connection) {
    std::cerr << "System Bus not available" << std::endl;
    exit(1);
  }

  this->m_proxy = Gio::DBus::Proxy::create_sync(
      connection, "org.freedesktop.UPower.PowerProfiles",
      "/org/freedesktop/UPower/PowerProfiles",
      "org.freedesktop.DBus.Properties");

  // Catch the PropertiesChanged signal
  this->m_signal_changed = this->m_proxy->signal_signal().connect(
          [this](Glib::ustring const& sender_name,
              Glib::ustring const& signal_name,
              Glib::VariantContainerBase const& parameters)
      {
        if ("PropertiesChanged" != signal_name) {
            // We are only interested in the PropertiesChanged signal
            std::cerr << "Unexpected signal: " << signal_name << std::endl;
            return;
        }

        if (3 != parameters.get_n_children()) {
            std::cerr << "Malformed PropertiesChanged notification, has " <<
                parameters.get_n_children() << " children." << std::endl;
            return;
        }

        Glib::Variant<Glib::ustring> owner_v;
        parameters.get_child(owner_v, 0);

        if (owner_v.get() != "org.freedesktop.UPower.PowerProfiles") {
            std::cerr << "Unexpected owner [" << owner_v.get() << "]" << std::endl;
            return;
        }

        Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>> active_profile_v;
        parameters.get_child(active_profile_v, 1);

        if (1 != active_profile_v.get().count("ActiveProfile")) {
            std::cerr << "Update did not include ActiveProfile; aborting." << std::endl;
            return;
        }

        Glib::Variant<Glib::ustring> profile_name_v =
            Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
                    active_profile_v.get()["ActiveProfile"]);

        std::cout << "Active Profile: " << profile_name_v.get() << std::endl;
        this->m_current_profile = profile_name_v.get();
        this->m_on_update(profile_name_v.get());
      });

    this->m_current_profile = this->get_profile();
}

PowerProfileManager::~PowerProfileManager() {}

void PowerProfileManager::set_on_update_dispatcher(std::function<void(Glib::ustring const&)> func) {
    this->m_on_update = func;
}

Glib::ustring PowerProfileManager::get_profile() {
  std::vector<Glib::VariantBase> params;
  params.push_back(Glib::Variant<Glib::ustring>::create(
      "org.freedesktop.UPower.PowerProfiles"));
  params.push_back(Glib::Variant<Glib::ustring>::create("ActiveProfile"));
  auto params_container = Glib::VariantContainerBase::create_tuple(params);
  auto result = this->m_proxy->call_sync("Get", params_container);

  assert_ambiguous_output(result.get_type_string() != "(v)", result, "");
  Glib::Variant<Glib::ustring> profile;
  assert_ambiguous_output(
      !Glib::VariantIter(result.get_child()).next_value(profile), result, "");
  return profile.get();
}

void PowerProfileManager::set_profile(Glib::ustring const& profile) {

  if (this->m_current_profile == profile) {
    return;
  }

  std::vector<Glib::VariantBase> params;
  params.push_back(Glib::Variant<Glib::ustring>::create(
      "org.freedesktop.UPower.PowerProfiles"));
  params.push_back(Glib::Variant<Glib::ustring>::create("ActiveProfile"));
  params.push_back(Glib::Variant<Glib::Variant<Glib::ustring>>::create(
      Glib::Variant<Glib::ustring>(
          Glib::Variant<Glib::ustring>::create(profile))));

  auto params_container = Glib::VariantContainerBase::create_tuple(params);
  auto result = this->m_proxy->call_sync("Set", params_container);
  assert_ambiguous_output(result.get_type_string() != "()", result, );
  assert_ambiguous_output(Glib::VariantIter(result).get_n_children() != 0,
                          result, );
}

std::vector<Glib::ustring> PowerProfileManager::get_all() {
  std::vector<Glib::ustring> profiles_available;

  std::vector<Glib::VariantBase> params;
  params.push_back(Glib::Variant<Glib::ustring>::create(
      "org.freedesktop.UPower.PowerProfiles"));
  auto params_container = Glib::VariantContainerBase::create_tuple(params);
  auto result = this->m_proxy->call_sync("GetAll", params_container);

  assert_ambiguous_output(result.get_type_string() != "(a{sv})", result,
                          profiles_available);

  Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>> main_struct;
  result.get_child(main_struct);
  assert_ambiguous_output(main_struct.get().count("Profiles") != 1, result,
                          profiles_available);

  Glib::VariantIter profiles((main_struct.get())["Profiles"]);
  Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>> profile;
  while (profiles.next_value(profile)) {
    assert_ambiguous_output(profile.get().count("Profile") != 1, result,
                            profiles_available);
    auto prof = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
        (profile.get())["Profile"]);
    profiles_available.push_back(prof.get());
  }

  return profiles_available;
}

