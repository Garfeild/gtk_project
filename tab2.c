#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "tab2.h"
#include "gui.h"

/* garfeild.c */

#define ROWS 7
#define COLUMNS 3

#define AVIABLE 0
#define UNAVIABLE 1

#define N_COLUMNS 16

#define MP_NAME 0
#define CORE_PERF 1
#define CORE_DIGIT 2
#define COUNT_TIMERS 3
#define ASYNC_PORT_TYPE 4
#define CASE_TYPE 5

#define GUARD_TIMER 6
#define INTERFACE_RAM 7
#define INTERFACE_DEBUG 8
#define DMA 9
#define PLL 10

#define ADC_DIGIT 11
#define ADC_CHANNELS 12
#define ADC_PERF 13

#define RAM_COMMAND 14
#define RAM_DATA 15 

#define OTHER 16
    
sqlite3 *db;
GtkTreeStore *store;

/* Callbacks */
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    NotUsed=0;
    GPtrArray *results;
    results = g_ptr_array_new();
    //g_print("Callback\n");
    for(i=0; i<argc; i++){
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        g_ptr_array_add(results, argv[i]);
     }
    //printf("\n");
    set_table_info(results);
    //g_free(results);

    return 0;
} 

//static int callbackForTable(void *NotUsed, int argc, char **argv, char **azColName)

void entry_print2(GtkWidget *gw, GPtrArray *array)
{ 
    char *zErrMsg = 0;
    int rc;
    int i;
    gboolean flag = TRUE;
    gchar *str2, *str3;
    //Подстроки запроса
    gchar *subStr[3] =  {
        "=\"", 
        "\"", 
        " AND "
    };
    
    gtk_tree_store_clear(store);
    //Дублируем str в str3
    str3 = g_strdup("SELECT mp_name, core_perf, core_digit,count_timers, async_port_type,case_type, guard_timer, interface_ram, interface_debug, dma, pll, adc_digit, adc_channels, adc_perf, ram_command, ram_data FROM mp WHERE ");
    str2 = g_strdup("");

    //Заполняем строку запроса
    for (i=0; i<array->len; i++)
    {  
        //g_print("%s\n", GTK_OBJECT_TYPE_NAME(g_ptr_array_index(array, i)));

        if ( GTK_WIDGET_SENSITIVE (g_ptr_array_index(array, i)) == TRUE && GTK_WIDGET_PARENT_SENSITIVE(g_ptr_array_index(array, i)) == TRUE )
            if(g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(array,i)), "GtkCheckButton") != 0)
        {
            //g_print("%s\n", GTK_OBJECT_TYPE_NAME(g_ptr_array_index(array,i)));
            ////Первая часть не содержить "AND"
            if (flag==FALSE)
            {
                    str2 = g_strconcat(str3, subStr[2], NULL);
                    str3 = g_strdup(str2);
            }
            else
                flag=FALSE;
            //Проверяем, какой тип виджета (для Ксю не обязательно) и активен ли i-ый виджет из массива. Если активен, до добавляем подстроку: 
            // " AND <название_столбца>=\"значение\""
            if ( g_strcmp0( GTK_OBJECT_TYPE_NAME( g_ptr_array_index( array, i ) ), "GtkEntry" ) == 0 )
                str2 = g_strconcat(
                        str3,                                                           //      Полученная ранее строка.
                        gtk_widget_get_name(g_ptr_array_index(array, i)),               //      имя виджета == имя столбца в базе
                        subStr[0],                                                      //      "=\""
                        gtk_entry_get_text(GTK_ENTRY(g_ptr_array_index(array,i))),      //      данные из виджета
                        subStr[1],                                                      //      "\""
                        NULL);
            else if (g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(array, i)), "GtkComboBox") == 0 && gtk_combo_box_get_active(g_ptr_array_index(array, i)) != -1 )
                str2 = g_strconcat(
                        str3, 
                        gtk_widget_get_name(g_ptr_array_index(array, i)), 
                        subStr[0], 
                        gtk_combo_box_get_active_text(GTK_COMBO_BOX(g_ptr_array_index(array,i))),
                        subStr[1], 
                        NULL);
            //Дублируем str2 в str3 для дальнейшей подстановки.
            str3 = g_strdup(str2); 
            //g_print("%s\n", str2);

        }
    }
    rc = sqlite3_exec(db, str2, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK )
    {
        g_fprintf(stderr,"SQL error: %s\n", zErrMsg);
    }
    
    //g_free(str);
    g_free(str2);
    g_free(str3);
}

void input_clear(GtkWidget *gw, GPtrArray *input)
{ 
    int i;

    for(i=0; i< input->len; i++)    {
        if( i<10)
            gtk_entry_set_text(GTK_ENTRY(g_ptr_array_index(input, i)), "");
        else if (i>=10 && i<input->len)
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ptr_array_index(input, i)), -1);
    }
}

void entry_enter(GtkWidget *gw, GtkWidget *button)
{
    g_signal_emit_by_name(G_OBJECT(button), "clicked");
}

/* table */
void set_column(GtkWidget *tree, const char *labelColumn[])
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  int i;
 
  for ( i=0; i < N_COLUMNS; i++ )
  {
 
          renderer = gtk_cell_renderer_text_new ();
          column = gtk_tree_view_column_new_with_attributes (labelColumn[i], renderer,
                  "text", i,
                  NULL);
 
      gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
  }
}
 
void set_table_info(GPtrArray *results)
{
    GtkTreeIter iter;
 
    gtk_tree_store_append (store, &iter, NULL); /* Acquire an iterator */
    gtk_tree_store_set (store, &iter,
            MP_NAME, g_ptr_array_index(results, MP_NAME),
            CORE_PERF, g_ptr_array_index(results ,CORE_PERF),                //1
            CORE_DIGIT,  g_ptr_array_index(results, CORE_DIGIT),            //2
            COUNT_TIMERS, g_ptr_array_index(results, COUNT_TIMERS),          //3
            ASYNC_PORT_TYPE, g_ptr_array_index(results, ASYNC_PORT_TYPE),   //4
            CASE_TYPE, g_ptr_array_index(results, CASE_TYPE),               //5
            GUARD_TIMER, g_ptr_array_index(results, GUARD_TIMER),           //6
            INTERFACE_RAM, g_ptr_array_index(results, INTERFACE_RAM),       //7
            INTERFACE_DEBUG, g_ptr_array_index(results, INTERFACE_DEBUG),   //8
            DMA, g_ptr_array_index(results, DMA),                           //9
            PLL, g_ptr_array_index(results, PLL),                           //0
            ADC_DIGIT, g_ptr_array_index(results, ADC_DIGIT),               //1
            ADC_CHANNELS, g_ptr_array_index(results, ADC_CHANNELS),         //2
            ADC_PERF, g_ptr_array_index(results, ADC_PERF),                 //3
            RAM_COMMAND, g_ptr_array_index(results, RAM_COMMAND),           //4
            RAM_DATA, g_ptr_array_index(results, RAM_DATA),                 //5
//            OTHER, g_ptr_array_index(results, OTHER),                       //6
            -1);
}

GtkWidget* setup_table(GtkTreeStore *store, const char *labelColumn[])
{
  GtkWidget *tree;
 
  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
 
  set_column(tree, labelColumn);
 
  g_object_unref (G_OBJECT (store));
 
  //g_print("setup\n");
  return tree;
 
}

GtkWidget* tab2()
{
    GtkWidget *tableBox;
    GtkWidget *button[2];
    GtkWidget *buttonBox;
    GtkWidget *tableOut;
    GtkWidget *scrWindow;
    //GtkTreeStore *store;
    GPtrArray *input;
    int i, j;

    const gchar *labelCoumn[17] = {
        "Модель",
        "Производительность\n ядра MIPS (оп/с)",                  //1
        "Разрядность ядра",                                     //2
        "Количество программируемых\n 32-х разрядных таймеров",   //3
        "Тип универсального\n асинхронного порта",                //4
        "Тип корпуса",                                          //5
        "Наличиие сторожевого\n таймера",                         //6
        "Наличие интерфейса\n внешней памяти",                    //7
        "Наличие отладочного\n интерфейса",                       //8
        "Наличие контроллера\n DMA",                              //9
        "Наличие PLL",                                          //0
        "АЦП: количество разрядов",                             //1
        "АЦП: количество каналов",                              //2
        "АЦП: скорость\n преобразования/производительность",      //3
        "Размер памяти команд (кбайт)",                         //4
        "Размер памяти данных(кбайт)",                           //5
        "Дополнительно"                                         //6
    };
    
    const gchar *nameEntries[5] = { 
        "Производительность ядра, MIPS (оп/с)",                              //1 
        "Разрядность ядра",                                     //2
        "Количество программируемых 32-х разрядных таймеров",   //3
        "Тип универсального асинхронного порта",                //4
        "Тип корпуса"                                           //5
    };
    
    const gchar *nameCombobox[5] = {
        "Наличиие сторожевого таймера",                         //1
        "Наличие интерфейса внешней памяти",                    //2
        "Наличие отладочного интерфейса",                       //3
        "Наличие контроллера DMA",                              //4
        "Наличие PLL"                                           //5
    };
    
    const gchar *namesACP[3] = {
        "АЦП: количество разрядов",                             //3
        "АЦП: количество каналов",                              //4
        "АЦП: скорость преобразования/производительность"       //5
    };
    
    const gchar *namesRAM[2] = {
        "Размер памяти команд, Кбайт",                                 //1
        "Размер памяти данных, Кбайт",                                  //2
    };
    //Названия кнопок
    const gchar *nameButton[2] = { "Поиск", "Сброс" };

    const gchar *namesFromBaseEntries[5] = {
        "core_perf",
        "core_digit",
        "count_timers",
        "async_port_type",
        "case_type"
    };

    const gchar *namesFromBaseCombobox[5] = {
        "guard_timer",
        "interface_ram",
        "interface_debug",
        "dma",
        "pll"
    };

    const gchar *namesFromBaseACP[3] = {
        "adc_digit",
        "adc_channels",
        "adc_perf"
    };

    const gchar *namesFromBaseRAM[2] = {
        "ram_command",
        "ram_data"
    };
    
    /*rc = sqlite3_open("base.db", &db);
    if( rc )   
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }*/
    
    //initiating buttons and labels
    buttonBox = gtk_hbutton_box_new();
    //Установка форматирования кнопок:
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
    //g_print("BUTTONS: Created\n");
    
    tableBox = gtk_table_new(ROWS, COLUMNS, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(tableBox), 5);
    gtk_table_set_col_spacings(GTK_TABLE(tableBox), 5);
    //g_print("TABLEBOX: Created\n");

    store = gtk_tree_store_new( 17,
            G_TYPE_STRING,  //0
            G_TYPE_STRING,  //1
            G_TYPE_STRING,  //2
            G_TYPE_STRING,  //3
            G_TYPE_STRING,  //4
            G_TYPE_STRING,  //5
            G_TYPE_STRING,  //6
            G_TYPE_STRING,  //7
            G_TYPE_STRING,  //8
            G_TYPE_STRING,  //9
            G_TYPE_STRING,  //0
            G_TYPE_STRING,  //1
            G_TYPE_STRING,  //2
            G_TYPE_STRING,  //3
            G_TYPE_STRING,  //4
            G_TYPE_STRING,  //5
            G_TYPE_STRING   //6
            );
    tableOut = setup_table(store, labelCoumn);

    scrWindow = gtk_scrolled_window_new (NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    input = g_ptr_array_new();
    //g_print("INPUT: Created\n");
    
    //results = g_ptr_array_new();

    for(j=0; j<2; j++)  {
        button[j] = gtk_button_new_with_label(nameButton[j]);
        gtk_widget_set_size_request(button[j], 70, 35);
        //g_print("BUTTON: Created\n");
    }
    
    for (i=0; i<3; i++) {
        if (i==1)   {
            gtk_table_attach_defaults(GTK_TABLE(tableBox), GTK_WIDGET(createFrame(input, "АЦП", namesACP, namesFromBaseACP, 3)), 1, 2, 0, 3 );

            gtk_table_attach_defaults(GTK_TABLE(tableBox), GTK_WIDGET(createFrame(input, "RAM", namesRAM, namesFromBaseRAM, 2)), 1, 2, 3, 5);
        }
        else if(i==0)
        {
            for(j=0; j<5; j++)
                gtk_table_attach_defaults(GTK_TABLE(tableBox), GTK_WIDGET(createEntry(input, nameEntries[j], namesFromBaseEntries[j])), i, i+1, j, j+1);
        }
        else if(i==2)
        {
            for(j=0; j<5; j++)
                gtk_table_attach_defaults(GTK_TABLE(tableBox), GTK_WIDGET(createComboBox(input, nameCombobox[j], namesFromBaseCombobox[j])), i, i+1, j, j+1);
        }
    }
    
    for(j=1; j>=0; j--)
        gtk_box_pack_start(GTK_BOX(buttonBox), button[j], FALSE, FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(tableBox), buttonBox, 2, 3, 5, 6);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrWindow), tableOut);
    gtk_table_attach_defaults(GTK_TABLE(tableBox), scrWindow, 0, 3, 6, 7);
    
    for(j=0; j<input->len; j++) { 
        if (j<10)   {
        g_signal_connect(G_OBJECT(g_ptr_array_index(input, j)), "activate",
                G_CALLBACK(entry_enter), (gpointer) button[0]);
        }
    }
   
    g_signal_connect(G_OBJECT(button[1]), "clicked", 
            G_CALLBACK(input_clear), (gpointer) input);

    g_signal_connect(G_OBJECT(button[0]), "clicked", 
            G_CALLBACK(entry_print2), (gpointer)input);
    
   gtk_container_set_border_width(GTK_CONTAINER(tableBox), 5);


   return tableBox;
}

/////////////////////////////////////////////////////////////////////////////
/* old stuff
 
GtkWidget *input[2];

void entry_print(GtkWidget *gw, GtkWidget *entry)
{ 
    char *zErrMsg = 0;
    int rc;
    //Массив частей запроса 'SELECT что-то FROM garfeild что-то-еще="значение";'
    const gchar *selectSubStr[4] = { "SELECT ", " FROM garfeild WHERE ", "=\"", "\";" };
    int k = 0;
    
    
    //DEBUG: Проверяем, действительно ли можно вычислить длинну.
    //g_fprintf(stderr, "%d\n",strlen(selectSubStr[0]));
    //g_fprintf(stderr, "%d\n",strlen(nameInput[TEXT]));
    //g_fprintf(stderr, "%d\n",strlen(selectSubStr[1]));
    //g_fprintf(stderr, "%d\n",strlen(gtk_widget_get_name(GTK_WIDGET(entry))));
    //g_fprintf(stderr, "%d\n",strlen(selectSubStr[2]));
    //g_fprintf(stderr, "%d\n",strlen(gtk_entry_get_text(GTK_ENTRY(entry))));
    //g_fprintf(stderr, "%d\n",strlen(selectSubStr[3])); 
    
    //k = длинна строки 'SELECT text FROM garfeild WHERE id="число";'
    //Сравниваем названия виджета entry с элементом массива nameInput, соответствующего ID.
    if ( g_strcmp0(gtk_widget_get_name(GTK_WIDGET(entry)),nameInput[ID]) == 0 )
    {
        //DEBUG: Проверка вхождения в цикл для первого if
        //g_fprintf(stderr, "ID!\n");
        k = strlen(selectSubStr[0]) + 
            strlen(nameInput[TEXT]) + 
            strlen(selectSubStr[1]) + 
            strlen(gtk_widget_get_name(GTK_WIDGET(entry))) + 
            strlen(selectSubStr[2]) + 
            strlen(gtk_entry_get_text(GTK_ENTRY(entry))) + 
            strlen(selectSubStr[3]); 
    }
    //Сравниваем названия виджета entry с элементом массива nameInput, соответствующего TEXT.
    else if ( g_strcmp0(gtk_widget_get_name(GTK_WIDGET(entry)),nameInput[TEXT]) == 0 )
    {
        //DEBUG
        //g_fprintf(stderr, "TEXT!\n");
        k = strlen(selectSubStr[0]) + 
            strlen(nameInput[ID]) + 
            strlen(selectSubStr[1]) + 
            strlen(gtk_widget_get_name(GTK_WIDGET(entry))) + 
            strlen(selectSubStr[2]) + 
            strlen(gtk_entry_get_text(GTK_ENTRY(entry))) + 
            strlen(selectSubStr[3]); 
    }
    //DEBUG: Проверяем, посчиталась ли длинна конечно строки.
    //g_fprintf(stderr, "%d\n", k);
    
    //Создаем указатель str на gchar длинной k. sizeof(gchar) для определения размера памяти под один элемент gchar.
    gchar *str = (gchar*)malloc(k*sizeof(gchar));
    //DEBUG: Проверка на падение после вызова malloc.
    //g_fprintf(stderr, "MALLOC!\n");
    
    //Устанавливаем нулевой размер для всех элементов str.
    memset(str, 0, k*sizeof(char));
    //DEBUG: Проверка на выполнение memset.
    //g_fprintf(stderr, "MEMSET\n");
    
    //Копируем первую часть запроса в конечную строку str.
    strcat(str, selectSubStr[0]);
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT1: %s\n", str);

    //Проеверяем, какой виджет entry используется, и копируем название другого виджета в str.
    if ( g_strcmp0(gtk_widget_get_name(GTK_WIDGET(entry)),nameInput[ID]) == 0 )
        strcat(str, nameInput[TEXT]);
    else if ( g_strcmp0(gtk_widget_get_name(GTK_WIDGET(entry)),nameInput[TEXT]) == 0 )
        strcat(str, nameInput[ID]);
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT2: %s\n", str);
    
    //Копируем вторую часть запроса в str.
    strcat(str, selectSubStr[1]);
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT3: %s\n", str);
    
    //Копируем название entry в запрос str.
    strcat(str, gtk_widget_get_name(GTK_WIDGET(entry)));
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT4: %s\n", str);
    
    //Копируем третью часть запроса в str.
    strcat(str, selectSubStr[2]);
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT5: %s\n", str);
    
    //Копируем текст из entry, который используем в качестве параметра запроса.
    strcat(str, gtk_entry_get_text(GTK_ENTRY(entry)));
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT6: %s\n", str);
    
    //Копируем четвертую, последнюю часть запроса в str.
    strcat(str, selectSubStr[3]);
    //DEBUG: Вывод полученной строки.
    //g_fprintf(stderr, "CAT7: %s\n", str);
    
    //Проверочный вывод полученно строки.
    g_print ("%s\n", str);
    rc = sqlite3_exec(db, str, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK )
    {
        g_fprintf(stderr,"SQL error: %s\n", zErrMsg);
    }
}

void entry_print2(GtkWidget *gw, gpointer array)
{ 
    char *zErrMsg = 0;
    int rc;
    //Массив частей запроса 'SELECT что-то FROM garfeild что-то-еще="значение";'
    //                                             
    const gchar *selectSubStr[4] = { 
        "SELECT id FROM garfeild WHERE ",   //0
        "=\"",                              //1
        "\" AND ",                          //2
        "\";"};                             //3
    int k = 0;
    
    GtkWidget *entry[2];
    entry[0] = gtk_entry_new();;
    
    //DEBUG: Проверяем, действительно ли можно вычислить длинну.
    g_fprintf(stderr, "%d\n",strlen(selectSubStr[0]));
    g_fprintf(stderr, "%d\n",strlen(selectSubStr[1]));
    g_fprintf(stderr, "%d\n",strlen(selectSubStr[2]));
    g_fprintf(stderr, "%d\n",strlen(selectSubStr[3])); 
    g_fprintf(stderr, "%d\n",strlen(gtk_widget_get_name(GTK_WIDGET(entry[NUMBER]))));
    g_fprintf(stderr, "%d\n",strlen(gtk_entry_get_text(GTK_ENTRY(entry[NUMBER]))));
    g_fprintf(stderr, "%d\n",strlen(gtk_widget_get_name(GTK_WIDGET(entry[TEXT]))));
    g_fprintf(stderr, "%d\n",strlen(gtk_entry_get_text(GTK_ENTRY(entry[TEXT]))));
    
    //k = длинна строки 'SELECT id FROM garfeild WHERE number="число" AND text="текст";'
    //Сравниваем названия виджета entry с элементом массива nameInput, соответствующего ID.
        //DEBUG: Проверка вхождения в цикл для первого if
        //g_fprintf(stderr, "ID!\n");
        k = strlen(selectSubStr[0]) + 
            strlen(gtk_widget_get_name(GTK_WIDGET(entry[NUMBER]))) + 
            2*strlen(selectSubStr[1]) + 
            strlen(gtk_entry_get_text(GTK_ENTRY(entry[NUMBER]))) + 
            strlen(selectSubStr[2]) +
            strlen(gtk_widget_get_name(GTK_WIDGET(entry[TEXT]))) +
            strlen(selectSubStr[3]);
    //DEBUG: Проверяем, посчиталась ли длинна конечно строки.
    g_fprintf(stderr, "%d\n", k);
    */
    /* strcat version

    //Создаем указатель str на gchar длинной k. sizeof(gchar) для определения размера памяти под один элемент gchar.
    gchar *str = (gchar*)malloc(k*sizeof(gchar));
    //DEBUG: Проверка на падение после вызова malloc.
    //g_fprintf(stderr, "MALLOC!\n");
    
    //Устанавливаем нулевой размер для всех элементов str.
     memset(str, 0, k*sizeof(char));
    //DEBUG: Проверка на выполнение memset.
    //g_fprintf(stderr, "MEMSET\n");
    
    //Копируем первую часть запроса в конечную строку str.
    strcat(str, selectSubStr[0]);
    
    strcat(str, gtk_widget_get_name(GTK_WIDGET(entry[NUMBER])));

    strcat(str, selectSubStr[1]);

    strcat(str, gtk_entry_get_text(GTK_ENTRY(entry[NUMBER])));

    strcat(str, selectSubStr[2]);

    strcat(str, gtk_widget_get_name(GTK_WIDGET(entry[TEXT])));

    strcat(str, selectSubStr[1]);

    strcat(str, gtk_entry_get_text(GTK_ENTRY(entry[TEXT])));

    strcat(str, selectSubStr[3]);
    */
    /* old stuff part 2
    gchar *str;

    str = g_strconcat(
            selectSubStr[0], 
            gtk_widget_get_name(GTK_WIDGET(entry[NUMBER])), 
            selectSubStr[1], 
            gtk_entry_get_text(GTK_ENTRY(entry[NUMBER])), 
            selectSubStr[2], 
            gtk_widget_get_name(GTK_WIDGET(entry[TEXT])), 
            selectSubStr[1], 
            gtk_entry_get_text(GTK_ENTRY(entry[TEXT])), 
            selectSubStr[3],
            NULL
            );


    //Проверочный вывод полученно строки.
    g_print ("%s\n", str);
    rc = sqlite3_exec(db, str, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK )
    {
        g_fprintf(stderr,"SQL error: %s\n", zErrMsg);
    }
    g_free(str);
}

void entry_clear(GtkWidget *gw, GtkWidget *entry)
{
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}
 */
/////////////////////////////////////////////////////////////////////////////
