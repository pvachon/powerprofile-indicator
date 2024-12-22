#ifndef APP_MENU_H
#define APP_MENU_H

#include "indicator.h"
#include <gtkmm/menu.h>
#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/radiomenuitem.h>

#include <functional>
#include <memory>

class AppMenu : public Gtk::Menu {
public:
  AppMenu(std::shared_ptr<Indicator> indicator, std::vector<Glib::ustring> const& profiles_available);
  virtual ~AppMenu();

  void update(Glib::ustring const& current_profile);
  void set_event_dispatcher(std::function<void(Glib::ustring const&)> on_update);

private:
  std::shared_ptr<Indicator> m_indicator;
  std::function<void(Glib::ustring const&)> m_on_update;

  Gtk::RadioButtonGroup m_group1;

  Gtk::RadioMenuItem m_power_saver_menu_item;
  Gtk::RadioMenuItem m_balanced_menu_item;
  Gtk::RadioMenuItem m_performance_menu_item;
};

#endif // !APP_MENU_H
