#include "app_menu.h"
#include <glibmm.h>
#include <gtkmm.h>

#include <iostream>

AppMenu::AppMenu(std::shared_ptr<Indicator> indicator,
        std::vector<Glib::ustring> const& profiles_available)
    : m_power_saver_menu_item("Power Saver"),
      m_balanced_menu_item("Balanced"),
      m_performance_menu_item("Performance"),
      m_group1(),
      m_on_update([](Glib::ustring const&) {})
{
  m_indicator = indicator;

  m_balanced_menu_item.set_group(m_group1);
  m_power_saver_menu_item.set_group(m_group1);
  m_performance_menu_item.set_group(m_group1);

  if (std::find(profiles_available.begin(), profiles_available.end(),
                "power-saver") != profiles_available.end())
    add(m_power_saver_menu_item);

  if (std::find(profiles_available.begin(), profiles_available.end(),
                "balanced") != profiles_available.end())
    add(m_balanced_menu_item);

  if (std::find(profiles_available.begin(), profiles_available.end(),
                "performance") != profiles_available.end())
    add(m_performance_menu_item);

  show_all();

  m_power_saver_menu_item.signal_activate().connect(
      [this]() {
        std::cout << "Click: power-saver" << std::endl;
        this->m_on_update("power-saver");
      });

  m_balanced_menu_item.signal_activate().connect(
      [this]() {
        std::cout << "Click: balanced" << std::endl;
        this->m_on_update("balanced");
      });

  m_performance_menu_item.signal_activate().connect(
      [this]() {
        std::cout << "Click: performance" << std::endl;
        this->m_on_update("performance");
      });
}

AppMenu::~AppMenu() {}

void AppMenu::set_event_dispatcher(std::function<void(Glib::ustring const&)> func)
{
    this->m_on_update = func;
}

void AppMenu::update(Glib::ustring const& current_profile) {
    std::cout << "Updating profile to " << current_profile << std::endl;
    if (current_profile == "balanced") {
        m_indicator.get()->change_icon("power-profile-balanced-symbolic");
        this->m_balanced_menu_item.set_active();
    } else if (current_profile == "power-saver") {
        m_indicator.get()->change_icon("power-profile-power-saver-symbolic");
        this->m_power_saver_menu_item.set_active();
    } else if (current_profile == "performance") {
        m_indicator.get()->change_icon("power-profile-performance-symbolic");
        this->m_performance_menu_item.set_active();
    }
}

