/* gui.h */
#ifndef  GUI_H
#define  GUI_H

#include <gtk/gtk.h>

/* функция для создания виджета GtkEntry с заданными параметрами и          *
 * необходимыми дополнительными виджетами.                                  *
 *                                                                          *
 * В функцию передаются следующие параметры:                                *
 * > GPtrArray *entries   -   массив указателей на виджеты. В него          *
 * записываются указатели всех создаваемых виджетов для ввода информации    *
 * > gchar *name          -   указатель на строку с текстом для обозначения *
 * виджета ввода (метки).                                                   *
 * > gchar *wname         -   указатель на строку с названием виджета.      *
 * Используется при составлении SQL запроса.                                *
 *                                                                          *
 * функция возвращает указатель на виджет, содержащий все необходимые       * 
 * виджеты                                                                  */
GtkWidget* createEntry(GPtrArray *entries, const gchar *name, const gchar *wname);

/* функция для создания виджета GtkComboBox с заданными параметрами и       *
 * необходимыми дополнительными виджетами.                                  *
 *                                                                          *
 * В функцию передаются следующие параметры:                                *
 * > GPtrArray *comboboxes    -   массив указателей на виджеты. В него      *
 * записываются указатели всех создаваемых виджетов для ввода информации    *
 * > gchar *name              -   указатель на строку с текстом для         *
 * обозначения виджета ввода (метки).                                       *
 * > gchar *wname             -   указатель на строку с названием виджета.  *
 * Используется при составлении SQL запроса.                                *
 *                                                                          *
 * функция возвращает указатель на виджет, содержащий все необходимые       * 
 * виджеты                                                                  */
GtkWidget* createComboBox(GPtrArray *comboboxes, const gchar *name, const gchar *wname);

/* функция для создания виджета GtkFrame с заданными параметрами и          *
 * необходимыми дополнительными виджетами.                                  *
 *                                                                          *
 * В функцию передаются следующие параметры:                                *
 * > GPtrArray *entries     -   массив указателей на виджеты. В него        *
 * записываются указатели всех создаваемых виджетов для ввода информации    *
 * > gchar *frameName       -   указатель на строку с текстом для метки     *
 * фрейма                                                                   *
 * > gchar *names[]         -   указатель на массив размерностью size,      *
 * который содержит строки для меток для каждого поля ввода                 *
 * > gchar *wnames[]        -   указатель на массив размерностью size,      *
 * который содержит строки для указания имени виджета (используется в       *
 * SQL запросах)                                                            *
 * > int size               -   содержит количество виджетов ввода во       *
 * фрейме                                                                   *
 *                                                                          * 
 * функция возвращает указатель на фрейм, содержащий size виджетов ввода    */
GtkWidget* createFrame(GPtrArray *entries, const gchar name[], const gchar *names[], const gchar *wnames[], const int size);

void show_dialog_info();

void clearTable(const int size, GtkTreeStore *store, void (*set_info)(GtkTreeStore*, GPtrArray*));

void entry_enter(GtkWidget *gw, GtkWidget *button);

#endif   
