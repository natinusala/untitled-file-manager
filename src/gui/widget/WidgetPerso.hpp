#ifndef WIDGETPERSO_HPP
#define WIDGETPERSO_HPP

#include <gtkmm.h>

namespace khanar
{
  class WidgetPerso
  {
  protected:
    void* content;

  public:
    void* getContent();
  };

  class GladeWidget : public WidgetPerso
  {

  };

  class ContentWidget : public WidgetPerso
  {

  };
}

#endif
