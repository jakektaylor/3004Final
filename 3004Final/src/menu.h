#ifndef MENU_H
#define MENU_H


#include <QString>
#include <QStringList>
#include <QVector>

/*  The Menu class is responsible for being a node in a tree of Menus that begins at the Main Menu which has 'prevMenu=NULL'.
    Each Menu has a list of items that are displayed in a QListWidget on the device.
*/
class Menu{
  
  public:
    Menu(QString name, QStringList list, Menu* prevMenu);
    ~Menu();
    
    //Getter methods
    QString getMenuName();
    QStringList getLists();
    Menu* getPrevMenu();
    Menu* getSubMenuAt(int index);

    //Setter methods
    void addSubMenu(Menu* menu);
    void addListItem(QString item);
    void removeitemAt(int index);
    void clear();
  
  private:
    QString menuName;           // The current menu that the user has selected
    QStringList items;          // The listed menu iteams on the menu section
    Menu* previousMenu;         // The main menu display of the device
    QVector<Menu*> subMenus;    // The sub menu option that found in a selected menu
    
};


#endif // MENU_H
