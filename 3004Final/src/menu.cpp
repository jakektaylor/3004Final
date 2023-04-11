#include "menu.h"

//Constructor and destructor
Menu::Menu(QString name, QStringList list, Menu* prevMenu){
  menuName = name;
  items = list;
  previousMenu = prevMenu;
  subMenus = QVector<Menu*>();
}

Menu::~Menu() {
    foreach(Menu* menu, subMenus) delete menu;
}

//Getter methods
QString Menu::getMenuName(){  return menuName; }
QStringList Menu::getLists(){ return items; }
Menu* Menu::getPrevMenu(){  return previousMenu; }
Menu* Menu::getSubMenuAt(int index) {
    if (index >=0 && index < subMenus.size()) return subMenus.at(index);
    return NULL;                    //Return NULL if an invalid index was passed in
}

//Setter methods
void Menu::addSubMenu(Menu *menu){subMenus.append(menu);}
void Menu::addListItem(QString itemName){this->items.append(itemName);}
void Menu::removeitemAt(int index){this->items.removeAt(index);}
void Menu::clear() {this->items.clear();}
