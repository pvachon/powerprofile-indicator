#include "app_menu.h"
#include "indicator.h"
#include "power_profile_manager.h"

#include <giomm.h>
#include <glibmm.h>
#include <gtkmm/application.h>
#include <mcheck.h>

#include <iostream>

int main(int argc, char *argv[]) {
  mtrace();

  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);

  std::shared_ptr<Indicator> indicator(new Indicator);
  PowerProfileManager manager;
  AppMenu app_menu(indicator, manager.get_all());
  app_menu.update(manager.get_profile());

  app_menu.set_event_dispatcher([&manager](Glib::ustring const& profile) {
            std::cout << "Setting profile: " << profile << std::endl;
            manager.set_profile(profile);
          });

  manager.set_on_update_dispatcher([&app_menu](Glib::ustring const& profile) {
            std::cout << "Notification of profile change: " << profile << std::endl;
            app_menu.update(profile);
          });

  indicator.get()->add_menu(app_menu);

  Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create();
  loop->run();

  muntrace();
  return 0;
}
