#include "gui.h"
/* gui.c */

/* toggle_action signal callback*/
/* функция-callback для обработки действий с GtkCheckButton             *
 *                                                                      *
 * В функцию передаются следующие параметры:                            *
 * > GtkWidget *checkButton     -   виджет, который произвел сигнал     *
 * > GtkWidget *widget          -   виджет, над которым нужно           *
 * произвести действие                                                  */
void toggle_action(GtkWidget *checkButton, GtkWidget *widget)
{
    // Переменная для хранения состояния виджета
    gboolean val;
 
    // Записываем состояние виджета GtkCheckButton в переменную val
    val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton));  
    // Проверяем, какое состояние имеет виджет GtkCheckButton, а так же проверяем активен ли виджет widget (может быть любой виджет, который "привязан" к GtkCheckButton  
    if( val == TRUE && !GTK_WIDGET_SENSITIVE(widget) )
    {
        // Если GtkCheckButton нажат, то widget должен быть активен.
        gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);                 
    }
    else if (val == FALSE)
    {   
        // Обратное действие
        gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
    }
}

/* createEntry */
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
GtkWidget* createEntry(GPtrArray *entries, const gchar *name, const gchar *wname)
{
    GtkWidget *_entry;      // Строка ввода
    GtkWidget *_checkButtonWLabel;      // GtkCheckButton с меткой для строки ввода
    GtkWidget *_leftAlign;  // Контейнер для форматирования по левому краю
    GtkWidget *_vbox;       // Вертикальный бокс
    
    // Создаем виджет GtkEntry
    _entry = gtk_entry_new();
    // Присваиваем имя виджета wname 
    gtk_widget_set_name(GTK_WIDGET(_entry), wname);
    // Добавляем в массив указателей.
    g_ptr_array_add(entries, _entry);
    
    // Создаем GtkCheckButton с меткой name 
    _checkButtonWLabel = gtk_check_button_new_with_label(name);
    // Устанавливаем GtkCheckButton в активное состояние
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_checkButtonWLabel), TRUE);
    
    // Определяем форматирование по левому краю
    _leftAlign = gtk_alignment_new(0, 0.5, 0.1, 0);
    
    // Создаем вертикальный бокс
    _vbox = gtk_vbox_new(FALSE, 5);
    
    // Добавляем GtkCheckButton в контейнер с форматированием по левому краю
    gtk_container_add(GTK_CONTAINER(_leftAlign), GTK_WIDGET(_checkButtonWLabel));
    
    // Последовательно добавляем виджеты в вертикальный бокс
    gtk_container_add(GTK_CONTAINER(_vbox), _leftAlign);
    gtk_container_add(GTK_CONTAINER(_vbox), _entry);

    // Соединяем сигнал снятия галочки для GtkCheckButton с виджетом ввода.
    g_signal_connect(G_OBJECT(_checkButtonWLabel), "toggled",
            G_CALLBACK(toggle_action), (gpointer) _entry);
    
    return _vbox;
}

/* createComboBox */
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
GtkWidget* createComboBox(GPtrArray *comboboxes, const gchar *name, const gchar *wname)
{
    GtkWidget *_combobox;                                   // Виджет выпадающего списка
    GtkWidget *_checkButtonWLabel;                          // GtkCheckButton с меткой
    GtkWidget *_leftAlign;                                  // Контейнер с форматированием по левому краю
    GtkWidget *_vbox;                                       // Вертикальный бокс
    const gchar *textForCombobox[2] = { "Есть", "Нет" };    // Список значений выпадающего списка
    
    _combobox = gtk_combo_box_new_text();
    // Присваиваем имя виджета wname 
    gtk_widget_set_name(GTK_WIDGET(_combobox), wname);
    gtk_combo_box_append_text(GTK_COMBO_BOX(_combobox), textForCombobox[0]); 
    gtk_combo_box_append_text(GTK_COMBO_BOX(_combobox), textForCombobox[1]); 
    // Добавляем в массив указателей.
    g_ptr_array_add(comboboxes, _combobox);
    
    // Создаем GtkCheckButton с меткой name 
    _checkButtonWLabel = gtk_check_button_new_with_label(name);

    // Определяем форматирование по левому краю
    _leftAlign = gtk_alignment_new(0, 0.5, 0.1, 0);
    
    // Создаем вертикальный бокс
    _vbox = gtk_vbox_new(FALSE, 5);
    
    // Добавляем GtkCheckButton в контейнер с форматированием по левому краю
    gtk_container_add(GTK_CONTAINER(_leftAlign), GTK_WIDGET(_checkButtonWLabel));
    
    // Последовательно добавляем виджеты в вертикальный бокс
    gtk_container_add(GTK_CONTAINER(_vbox), _leftAlign);
    gtk_container_add(GTK_CONTAINER(_vbox), _combobox);

    // Соединяем сигнал снятия галочки для GtkCheckButton с виджетом ввода.
    g_signal_connect(G_OBJECT(_checkButtonWLabel), "toggled",
            G_CALLBACK(toggle_action), (gpointer) _combobox);
    // Вызываем сигнал toggled для виджета _checkButtonWLabel, чтобы сработал
    // toggle_action. Необходимо для установки виджета _combobox в неактивное
    // состояние.
    g_signal_emit_by_name(_checkButtonWLabel, "toggled", (gpointer)_combobox);
    
    return _vbox;
}

/* createFrame */
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
GtkWidget* createFrame(GPtrArray *entries, const gchar *frameName, const gchar *names[], const gchar *wnames[], const int size)
{
    GtkWidget *_frame;              // фрейм
    GtkWidget *_checkButtonWLabel;  // GtkCheckButton с меткой
    GtkWidget *_vbox;               // Вертикальный бокс
    int i;                          

    // Создаем пустой фрейм
    _frame = gtk_frame_new(NULL);
    // Создаем вертикальный бокс
    _vbox = gtk_vbox_new(FALSE, 5);
    // Создаем GtkCheckButton с меткой frameName
    _checkButtonWLabel = gtk_check_button_new_with_label(frameName);
    // Устанавливаем GtkCheckButton в активное состояние
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_checkButtonWLabel), TRUE);
    // Устанавливаем GtkCheckButton в качестве метки фрейма
    gtk_frame_set_label_widget(GTK_FRAME(_frame), GTK_WIDGET(_checkButtonWLabel));
    // Заполняем фрейм полями ввода 
    for ( i=0; i<size; i++ )
    {   
        // Вызываем функцию createEntry для создания виджета ввода с меткой и
        // GtkCheckButton
        gtk_container_add(GTK_CONTAINER(_vbox), createEntry(entries, names[i], wnames[i]));

    }

    // Добавляем в вертикальный бокс фрейм
    gtk_container_add(GTK_CONTAINER(_frame), GTK_WIDGET(_vbox));
    gtk_container_set_border_width(GTK_CONTAINER(_frame), 5);
    
    // Соединяем сигнал снятия галочки с GtkCheckButton с вертикальным боксом.
    // Вообще _vbox заведен для того, чтобы не блокировался сам фрейм и _checkButtonWLabel
    g_signal_connect(G_OBJECT(_checkButtonWLabel), "toggled",
            G_CALLBACK(toggle_action), (gpointer) _vbox);
   
    return _frame;
}
