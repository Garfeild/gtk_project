/*tab3.c*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "tab1.h"
#include "gui.h"

sqlite3 *db;
GPtrArray *res1;
GPtrArray *stores;
GPtrArray *empty;
GArray *sizes;
int index_of_store;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    //g_print("CALLBACK\n");
    int i, j;
    NotUsed=0;
    gboolean alreayInArray = FALSE;
    
    for(i=0; i<argc; i++)
    {
        alreayInArray = FALSE;
        //g_printf("%s\n",argv[i]);
        for ( j=0; j<res1->len; j++ )
            if (g_strcmp0(g_ptr_array_index(res1, j), argv[i]) == 0 )
                alreayInArray = TRUE;

        if ( alreayInArray == FALSE )
        {
            g_printf("IF | %s\n", argv[i]);
            g_ptr_array_add(res1, g_strdup(argv[i]));
        }

    }

    return 0;
}

static int callback2(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    NotUsed=0;
    GPtrArray *results;
    results = g_ptr_array_new();

    for(i=0; i<argc; i++)
    {
        g_ptr_array_add(results, argv[i]);
        ////g_print("CALLBACK:\t%s\n", (gchar*)g_ptr_array_index(res1, res1->len-1));
    }

    set_table_info_xu(g_ptr_array_index(stores, index_of_store), results);
    return 0;
} 

static void sch_callback( GtkWidget *widget, GPtrArray *arr) 
{ 
    GtkWidget *tempPtr;
    char *zErrMsg = 0;
    GPtrArray *tempRes, *tempRes2;
    int rc, i, j, k;
    gchar *str1, *str2;
    gchar *search_template[] = 
    {
            "SELECT MODEL, phase_number, U_in, freq_in FROM input WHERE model=\"",

            "SELECT model, power_min, power_max, phase_number, range_reg_U_min, range_reg_U_max, diskr_reg_U, range_reg_f_min, range_reg_f_max, diskr_reg_f, range_reg_v_razsys_min, range_reg_v_razsys_max, range_reg_v_zamsys_min, range_reg_v_zamsys_max, control_way, mod_way, takt_f FROM output WHERE model=\"",


             "SELECT model, zad_freq_tp, ustan_zad_freq_tp, command_pusk_stop, pusk_stop_har, auto_pusk_stop_error, reg_techn_var FROM work_functions WHERE model=\"",


             "SELECT model, defend_from_kz, max_i_defend, obriv_perekos_phase_defend, inc_decr_U_in_i_const_zveno FROM defend_functions4 WHERE model=\"", 


             "SELECT model, power_leave_MP_defend, teplo_defend  FROM defend_functions2 WHERE model=\"", 


             "SELECT model, k_U_out_depend_U_in, k_int_i_drive, k_int_torm_U_lim FROM korr WHERE model=\"" 
    };
    gboolean BEGIN_FLAG = TRUE;
    gboolean CALLBACK_FLAG = TRUE;
    str1 = g_strdup("");
    str2 = g_strdup("");
    /************ ===== SQL EXAMPLE FOR FUTURE ====> ***********
     * select <table_name_1>.model from <table_name_1> 
     * join <table_name_2> on korr.model=<table_name_2>.model
     * join <table_name_3> on <table_name_2>.model=<table_name_3>.model
     * where k_int_i_drive="нет" and takt_f="15" and freq_in="50"
     * ********************************************************/

    g_ptr_array_free(res1, TRUE);
    res1 = g_ptr_array_new();

    tempRes = g_ptr_array_new();
    tempRes2 = g_ptr_array_new();


    // Первый вызов - ищем модели, подходящие под введенные параметры.

    for( i=0; i<arr->len; i++ ) {
        if(g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(arr,i)), "GtkCheckButton") == 0)    {

            if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_ptr_array_index(arr, i))) == TRUE ) {
                str1 = g_strconcat("SELECT MODEL FROM ", gtk_widget_get_name(GTK_WIDGET(g_ptr_array_index(arr, i))), " WHERE ", NULL );

                for( j=i+1; j<arr->len && g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(arr,j)), "GtkCheckButton")!=0; j++ ) {
                    tempPtr = g_ptr_array_index(arr, j);

                    if ( GTK_WIDGET_SENSITIVE (tempPtr) == TRUE && GTK_WIDGET_PARENT_SENSITIVE(tempPtr) == TRUE )   {

                        if ( BEGIN_FLAG == FALSE )   {
                            str2 = g_strconcat(str1, " AND ", NULL);
                            g_free(str1);
                            str1 = g_strdup(str2);
                            g_free(str2);
                        }   
                        else 
                            BEGIN_FLAG = FALSE;

                        if( g_strcmp0(GTK_OBJECT_TYPE_NAME(tempPtr), "GtkEntry") == 0 )
                            str2 = g_strconcat(str1, 
                                    gtk_widget_get_name(GTK_WIDGET(tempPtr)),
                                    "\"",
                                    gtk_entry_get_text(GTK_ENTRY(tempPtr)),
                                    "\"",
                                    NULL
                                    );
                        else if( g_strcmp0(GTK_OBJECT_TYPE_NAME(tempPtr), "GtkComboBox") == 0 )
                            str2 = g_strconcat(str1, 
                                    gtk_widget_get_name(GTK_WIDGET(tempPtr)),
                                    "\"",
                                    gtk_combo_box_get_active_text(GTK_COMBO_BOX(tempPtr)),
                                    "\"",
                                    NULL
                                    );

                        g_free(str1);
                        str1 = g_strdup(str2);
                        g_free(str2);
                        //g_printf("%s\n", str1);
                    }
                }

                rc = sqlite3_exec(db, str1, callback, 0, &zErrMsg);
                if( rc!=SQLITE_OK ) {
                    g_fprintf(stderr,"SQL error: %s\n", zErrMsg);
                }

                g_free(str1);
                BEGIN_FLAG = TRUE;

                if ( CALLBACK_FLAG == TRUE )
                {
                    for( k=0; k<res1->len; k++ )
                        g_ptr_array_add( tempRes, g_strdup( (gchar*)g_ptr_array_index( res1, k ) ) );

                    g_ptr_array_free(res1, TRUE);
                    res1 = g_ptr_array_new();
                    
                    CALLBACK_FLAG = FALSE;
                }
                else 
                {
                    for ( k=0; k<res1->len; k++ )
                        for ( j=0; j<tempRes->len; j++ )
                            if ( g_strcmp0( (gchar*)g_ptr_array_index( res1, k ), (gchar*)g_ptr_array_index( tempRes, j ) ) == 0 )
                                g_ptr_array_add( tempRes2, g_strdup( (gchar*)g_ptr_array_index( tempRes, j ) ) );

                    g_ptr_array_free(tempRes, TRUE);
                    tempRes = g_ptr_array_new();

                    for ( j=0; j<tempRes2->len; j++ )
                                g_ptr_array_add( tempRes, g_strdup( (gchar*)g_ptr_array_index( tempRes2, j ) ) );

                    g_ptr_array_free(tempRes2, TRUE);
                    tempRes2 = g_ptr_array_new();
                    g_ptr_array_free(res1, TRUE);
                    res1 = g_ptr_array_new();
                }

            }

        }

    }

    //for ( i=0; i<tempRes->len; i++ )
        //g_printf("#%d, %s\n", i, (gchar*)g_ptr_array_index(tempRes, i));

    // Второй вызов - собираем всю-всю инфу из базы для найденных моделей
    if ( tempRes->len == 0 )
    {
        for( i=0; i<stores->len && i<sizes->len; i++ )
        {
            clear_table( g_array_index(sizes, int, i), g_ptr_array_index(stores, i) );
        }
        show_dialog_info();
    }
    else
    {
        for ( i=0; i<stores->len; i++ )
            gtk_tree_store_clear(g_ptr_array_index(stores, i));

                index_of_store = 0;
        for ( i=0; i<arr->len; i++)
            if(g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(arr,i)), "GtkCheckButton") == 0)    {
                
                    for ( j=0; j<tempRes->len; j++) 
                    {

                        str1 = g_strconcat(search_template[index_of_store], g_ptr_array_index(tempRes, j), "\"", NULL );
                        //g_printf("Second: %s\n", str1);
                        rc = sqlite3_exec(db, str1, callback2, 0, &zErrMsg);
                        if( rc!=SQLITE_OK ) {
                            g_fprintf(stderr,"SQL error: %s\n", zErrMsg);
                        }
                    
                    g_free(str1);
                    }
                    index_of_store++;

            }

    }
    g_ptr_array_free(tempRes, TRUE);
    g_ptr_array_free(tempRes2, TRUE);

}

static void clr_callback( GtkWidget *widget, GPtrArray *array )
{
    int i;
    for (i=0; i < array->len; i++)
    {
        if (g_strcmp0 (GTK_OBJECT_TYPE_NAME (g_ptr_array_index(array, i)), "GtkEntry") == 0)
        {
            gtk_entry_set_text (GTK_ENTRY (g_ptr_array_index(array, i)), "");
        }
        else if (g_strcmp0 (GTK_OBJECT_TYPE_NAME (g_ptr_array_index(array, i)),"GtkComboBox") == 0)
            gtk_combo_box_set_active (GTK_COMBO_BOX(g_ptr_array_index(array, i)), -1);
    }
        
    for( i=0; i<stores->len && i<sizes->len; i++ )
        {
            clear_table( g_array_index(sizes, int, i), g_ptr_array_index(stores, i) );
        }
}

static void state_callback( GtkToggleButton *checkbutton, GtkWidget *entry )
{
    gboolean val;
    val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
    if (val == TRUE)
    {
        //button is pressed
        gtk_widget_set_sensitive (GTK_WIDGET(entry), TRUE);
    }
    else if (val == FALSE)
    {
        //button is raised
        gtk_widget_set_sensitive (GTK_WIDGET(entry), FALSE);

    }
    
}

void set_column_xu(GtkWidget *tree, const char *labelColumn[], int size)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  int i;
 
  for ( i=0; i < size; i++ )
  {
 
          renderer = gtk_cell_renderer_text_new ();
          //g_object_set(G_OBJECT(renderer), "alignment", PANGO_ALIGN_CENTER, NULL);
          column = gtk_tree_view_column_new_with_attributes (labelColumn[i], renderer,
                  "text", i,
                  NULL);
 
      gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
  }
}
 
void set_table_info_xu(GtkTreeStore *store, GPtrArray *results)
{
    GtkTreeIter iter;
    int i;
    //g_print("*****************Setting table******************\n");   
    gtk_tree_store_append (store, &iter, NULL); 
    for (i=0; i<results->len; i++)
    {
        //g_print("%s\n", (gchar*) g_ptr_array_index(results, i));
        //gtk_tree_store_append (store, &iter, NULL); 
        gtk_tree_store_set (store, &iter, i, g_ptr_array_index(results, i), -1);
    }
}

void clear_table(const int size, GtkTreeStore *store)
{
    GPtrArray *empty = g_ptr_array_new();
    int k;

    gtk_tree_store_clear(store);
    
    for (k=0; k < size; k++)
    {
        g_ptr_array_add(empty, "");
    }

    for (k=0; k < 3; k++) {
        set_table_info_xu (store, empty);
    }
    g_ptr_array_free (empty, FALSE);
}

GtkWidget* setup_table_xu(GtkTreeStore *store, const gchar *labelColumn[], int size)
{
  GtkWidget *tree;
 
  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
 
  set_column_xu(tree, labelColumn, size);
 
  //g_object_unref (G_OBJECT (store));
 
  //g_print("setup\n");
  return tree;
 
}


/***********************************************************************/
GtkWidget* tab1 ()
{   
    GtkWidget *table; //таблица для frame`ов
    GtkWidget *table_in; 
    GtkWidget *entry;
    GtkWidget *sch_button, *clr_button;
    GtkWidget *frame;
    GtkWidget *bbox; //button box
    GtkWidget *checkbutton; //enable\disable entries
    GtkWidget *hbox, *vbox; //for ch_boxes
    GtkWidget *combo;
    GtkWidget *notebook;
    GtkWidget *tableOut;
    GtkWidget *scr_window;
    GtkTreeStore *tempStore;
    GPtrArray *array;
    int i,j,m,n,k;
    int size;
    // Загловки
    gchar *frame_name[] = {
        "Входные параметры",
        "Выходные параметры",
        "Рабочие функции",
        "Защиты преобразователя\nи двигателя", 
        "Защиты от неправильной\nработы тиристорного выпрямителя" ,
        "Режимы коррекции"};

    gchar *input_entry_name[]={
        "Число фаз",
        "Напряжение питающией сети,В", 
        "Частота питающей сети,Гц"};

    gchar *output_entry_name[]={
        // Столбец 1
        "Мощность(max),кВт", 
        
        "Диапазон регулирования\nнапряжения(верхняя граница),В", 
       
        "Диапазон регулирования\nчастоты(верхняя граница),Гц",
        
        "Диапазон регулирования\nскорости в разомкнутой\nсистеме(верхняя граница)",
        
        // Столбец 2
        "Мощность(min),кВт", 
        
        "Диапазон регулирования\nнапряжения(нижняя граница),В", 
        
        "Диапазон регулирования\nчастоты(нижняя граница),Гц",
        
        "Диапазон регулирования\nскорости в разомкнутой\nсистеме(нижняя граница)",
        
        // Столбец 3
        "Число фаз(1 или 3)",
        
        "Дискретность регулирования\nнапряжения,В",
        
        "Дискретность регулирования\nчастоты,Гц",
        
        "Диапазон регулирования\nскорости в замкнутой\nсистеме(верхняя граница)",
        
        // Столбец 4
        "Принцип управления", 
        
        "Способ модуляции", 
        
        "Тактовая частота ШИМ,кГц",
        
        "Диапазон регулирования\nскорости в замкнутой\nсистеме(нижняя граница)",
        
       };

    gchar *work_name[]={
        "Задание частоты или\nтехнологической переменной",
        "Установки задания частоты или\nтехнологической переменной",
        "Команды пуск/стоп","Пуско-тормозная характеристика",
        "Автоматический повторный пуск\nпри обнаружении ошибки","Регулятор технологической переменной"};

    gchar *defend_title[]={
        "Защиты преобразователя и двигателя",
        "Защита от неправильной работы тиристорного выпрямителя"};

    gchar *defend1_name[]={
        "Защита от короткого\nзамыкания на корпус",
        "Максимально-токовая защита",
        "Защита от обрыва\nи перекоса фаз",
        "Защита от понижения или повышения\nнапряжения в звене постоянного тока"};
    
    gchar *defend2_name[]={
        "Тепловая защита",
        "Защита от потери\nпитания контроллером"};
    
    gchar *korr_name[] = {
        "Коррекция выходного напряжения\nв зависимости от напряжения питающей сети",
        "Коррекция интенсивности(при разгоне) и рабочей частоты\n(в установившимся режиме) при превышении тока двигетеля",
        "Коррекция интенсивности торможения при\nпревышении напряжения на звене постоянного тока"};

    // Имена виджетов
    gchar *base_frame[] = {
        "input",
        "output",
        "work_functions",
        "defend_functions4",
        "defend_functions2",
        "korr"
    };

    gchar *base_input[] = {
        "phase_number=",
        "U_in=",
        "freq_in="};

    gchar *base_output[] = {
        // Столбец 1
        "power_max>=",

        "range_reg_U_max>=",

        "range_reg_f_max>=",
 
        "range_reg_v_razsys_max>=",
        
        // Столбец 2
        "power_min<=",

        "range_reg_U_min<=",

        "range_reg_f_min<=",

        "range_reg_v_razsys_min<=",
        
        // Столбец 3
        "phase_number=",

        "diskr_reg_U=",

        "diskr_reg_f=",
        
        "range_reg_v_zamsys_max<=",
    
        // Столбец 4
        "control_way=", 
        
        "mod_way=", 
        
        "takt_f=",
        
        "range_reg_v_zamsys_min>=",
        
    };

    gchar *base_work[] = {
        "zad_freq_tp=",
        "ustan_zad_freq_tp=", 
        "command_pusk_stop=", 
        "pusk_stop_har=",
        "auto_pusk_stop_error=",
        "reg_techn_var="};

    gchar *base_defend2[] = {
        "power_leave_MP_defend=",
        "teplo_defend="};
    
    gchar *base_defend4[] = {
        "inc_decr_U_in_i_const_zveno=",
        "obriv_perekos_phase_defend=",
        "defend_from_kz=",
        "max_i_defend="};
    
    gchar *base_korr[] = {
        "k_int_i_drive=",
        "k_int_torm_U_lim=",
        "k_U_out_depend_U_in="};
    
    //Имена для столбцов таблицы-вывода
    const gchar *input_entry_name_table[]={
        "Модель\n", 
        "Число фаз\n",
        "Напряжение питающией сети\nВ", 
        "Частота питающей сети\nГц"};

    const gchar *output_entry_name_table[]={
        "Модель\n",                                                   //0 
        "Мощность\nmin, кВт",                                       //1
        "Мощность\nmax, кВт",                                       //2
        "Число фаз\n",                                                //3
        "Диапазон регулирования\nнапряжени min, В",                 //4
        "Диапазон регулирования\nнапряжени max, В",               //5
        "Дискретность регулирования\nнапряжения %",                    //6
        "Диапазон регулирования\nчастоты min, Гц",                           //7
        "Диапазон регулирования\nчастоты max, Гц",                           //8
        "Дискретность регулирования\nчастоты %",                       //9
        "Диапазон регулирования скорости\nв разомкнутой системе min",    //0
        "Диапазон регулирования скорости\nв разомкнутой системе max",    //1
        "Диапазон регулирования скорости\nв замкнутой системе nmin",      //2
        "Диапазон регулирования скорости\nв замкнутой системе max",      //3
        "Принцип управления\n",                                       //5
        "Способ модуляции\n",                                         //6
        "Тактовая частота ШИМ\n"                                      //7
    };
 
    const gchar *work_name_table[]={
        "Модель\n", 
        "Задание частоты или\nтехнологической переменной",
        "Установки задания частоты\nили технологической переменной",
        "Команды пуск/стоп\n","Пуско-тормозная характеристика\n",
        "Автоматический повторный пуск\nпри обнаружении ошибки","Регулятор технологической переменной\n"};

    const gchar *defend1_name_table[]={
        "Модель\n", 
        "Защита от короткого\nзамыкания на корпус",
        "Максимально-токовая защита\n","Защита от обрыва и перекоса фаз\n",
        "Защита от понижения или повышения\nнапряжения в звене постоянного тока"};
 
    const gchar *defend2_name_table[]={
        "Модель\n", 
        "Тепловая защита\n",
        "Защита от потери\nпитания контроллером"};

    const gchar *korr_name_table[] = {
        "Модель\n", 
        "Коррекция выходного напряжения\nв зависимости от напряжения питающей сети",
        "Коррекция интенсивности(при разгоне) и рабочей частоты\n(в установившимся режиме) при превышении тока двигетеля",
        "Коррекция интенсивности торможения при\nпревышении напряжения на звене постоянного тока"};

    table = gtk_table_new (15, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    notebook = gtk_notebook_new();
    array = g_ptr_array_new ();
    res1 = g_ptr_array_new();
    for (j=0; j<13; j++)
    {
        if (j==0) //input
        {
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label (frame_name[0]);
            gtk_widget_set_name(GTK_WIDGET(checkbutton), base_frame[0]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 0, 1);
            table_in = gtk_table_new (1, 3, TRUE); //false
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 100);//5
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 100);//5
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            for(m=0; m<3; m++)
            {
                entry = gtk_entry_new ();
                gtk_entry_set_max_length(GTK_ENTRY(entry), 5);
                gtk_widget_set_name (GTK_WIDGET(entry), base_input[m]);
                g_ptr_array_add (array, (gpointer) entry);
                vbox = gtk_vbox_new (FALSE, 5);
                checkbutton = gtk_check_button_new_with_label (input_entry_name[m]);
                //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                g_signal_connect (G_OBJECT (checkbutton), "toggled",
                        G_CALLBACK (state_callback), (gpointer) entry);
                // Уберем все галки
                g_signal_emit_by_name(checkbutton, "toggled", (gpointer) entry);
                gtk_container_add (GTK_CONTAINER(vbox), checkbutton);
                gtk_container_add (GTK_CONTAINER(vbox), entry);
                gtk_table_attach_defaults (GTK_TABLE (table_in), vbox, 0+m, 1+m, 0, 1);
            }
        }  

        if (j==1) //output
        {
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label (frame_name[1]);
            gtk_widget_set_name(GTK_WIDGET(checkbutton), base_frame[1]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 1, 4);
            table_in = gtk_table_new (4, 4, FALSE);//4 3
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 5);
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 5);
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            k=0;
            for(m=0; m<4; m++) //3
                for(n=0; n<4; n++)
                {
                    //printf ("n=%d\n", n);
                    //printf ("m=%d\n", m);
                    entry = gtk_entry_new ();
                    g_ptr_array_add (array, (gpointer) entry);
                    gtk_widget_set_name (GTK_WIDGET(entry), base_output[k]);
                    vbox = gtk_vbox_new (FALSE, 5);
                    checkbutton = gtk_check_button_new_with_label (output_entry_name[k]);
                    //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                    g_signal_connect (G_OBJECT (checkbutton), "toggled",
                            G_CALLBACK (state_callback), (gpointer) entry);
                // Уберем все галки
                    g_signal_emit_by_name(checkbutton, "toggled", (gpointer) entry);
                    gtk_container_add (GTK_CONTAINER(vbox), checkbutton);
                    gtk_container_add (GTK_CONTAINER(vbox), entry);
                    gtk_table_attach_defaults (GTK_TABLE (table_in), vbox, 0+m, 1+m, 0+n, 1+n);
                    k++;
                }
        } 

        if (j==5) //work
        {
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label (frame_name[2]);
            gtk_widget_set_name(GTK_WIDGET(checkbutton), base_frame[2]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 5, 7);
            table_in = gtk_table_new (2, 3, FALSE);
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 5);
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 5);
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            k=0;
            for(m=0; m<2; m++)
                for(n=0; n<3; n++)
                {
                    hbox = gtk_hbox_new (FALSE, 5);
                    combo = gtk_combo_box_new_text();
                    gtk_widget_set_name (GTK_WIDGET(combo), base_work[k]);
                    g_ptr_array_add (array, (gpointer) combo);
                    if(g_strcmp0(work_name[k], "Пуско-тормозная характеристика" ) == 0) 
                    {
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "S-образная/Линейная");
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Другая");
                    }
                    else
                    {
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "есть");
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "нет");
                    }
                    checkbutton = gtk_check_button_new_with_label (work_name[k]);
                    //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                    g_signal_connect (G_OBJECT (checkbutton), "toggled",
                            G_CALLBACK (state_callback), (gpointer) combo);
                // Уберем все галки
                g_signal_emit_by_name(checkbutton, "toggled", (gpointer) combo);
                    gtk_container_add (GTK_CONTAINER(hbox), checkbutton);
                    gtk_container_add (GTK_CONTAINER(hbox), combo);
                    gtk_table_attach_defaults (GTK_TABLE (table_in), hbox, 0+m, 1+m, 0+n, 1+n);
                    k++;
                }
        }
       
        if (j==7) //defend4
        { 
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label (defend_title[0]);
            gtk_widget_set_name (GTK_WIDGET(checkbutton), base_frame[3]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 7, 9);
            table_in = gtk_table_new (2, 2, FALSE);
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 5);
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 5);
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            k=0;
            for(m=0; m<2; m++)
                for(n=0; n<2; n++)
                {
                    hbox = gtk_hbox_new (FALSE, 50);
                    combo = gtk_combo_box_new_text();
                    gtk_widget_set_name (GTK_WIDGET(combo), base_defend4[k]);
                    g_ptr_array_add (array, (gpointer) combo);
                    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "есть");
                    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "нет");
                    checkbutton = gtk_check_button_new_with_label (defend1_name[k]);
                    //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                    g_signal_connect (G_OBJECT (checkbutton), "toggled",
                            G_CALLBACK (state_callback), (gpointer) combo);
                    // Уберем все галки
                    g_signal_emit_by_name(checkbutton, "toggled", (gpointer) combo);
                    gtk_container_add (GTK_CONTAINER(hbox), checkbutton);
                    gtk_container_add (GTK_CONTAINER(hbox), combo);
                    gtk_table_attach_defaults (GTK_TABLE (table_in), hbox, 0+m, 1+m, 0+n, 1+n);
                    k++;
                }
        }

        if (j==9) //defend2
        { 
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label (defend_title[1]);
            gtk_widget_set_name(GTK_WIDGET(checkbutton), base_frame[4]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 9, 11);
            table_in = gtk_table_new (1, 2, FALSE);
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 5);
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 5);
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            for(m=0; m<2; m++)
            {
                hbox = gtk_hbox_new (FALSE, 5);
                combo = gtk_combo_box_new_text();
                g_ptr_array_add (array, (gpointer) combo);
                gtk_widget_set_name (GTK_WIDGET(combo), base_defend2[m]);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "есть");
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "нет");
                checkbutton = gtk_check_button_new_with_label (defend2_name[m]);
                //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                g_signal_connect (G_OBJECT (checkbutton), "toggled",
                        G_CALLBACK (state_callback), (gpointer) combo);
                // Уберем все галки
                g_signal_emit_by_name(checkbutton, "toggled", (gpointer) combo);
                gtk_container_add (GTK_CONTAINER(hbox), checkbutton);
                gtk_container_add (GTK_CONTAINER(hbox), combo);
                gtk_table_attach_defaults (GTK_TABLE (table_in), hbox, 0+m, 1+m, 0, 1);
            }
        }

        if (j==11) //korr
        { 
            frame = gtk_frame_new(NULL);
            checkbutton = gtk_check_button_new_with_label ("Режимы коррекции");
            gtk_widget_set_name(GTK_WIDGET(checkbutton), base_frame[5]);
            g_ptr_array_add (array, (gpointer) checkbutton);
            gtk_frame_set_label_widget(GTK_FRAME(frame), checkbutton);
            gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 3, 11, 13);
            table_in = gtk_table_new (2, 2, FALSE);
            gtk_table_set_row_spacings(GTK_TABLE(table_in), 5);
            gtk_table_set_col_spacings(GTK_TABLE(table_in), 5);
            gtk_widget_set_sensitive(GTK_WIDGET(table_in), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), FALSE);
            g_signal_connect (G_OBJECT (checkbutton), "toggled",
                    G_CALLBACK (state_callback), (gpointer) table_in);
            gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
            gtk_container_add (GTK_CONTAINER(frame), table_in);
            k=0;
            for(m=0; m<2; m++)
                for(n=0; n<2; n++)
                    if (n!=1 || m!=1)

                    {
                        hbox = gtk_hbox_new (FALSE, 5);
                        combo = gtk_combo_box_new_text();
                        g_ptr_array_add (array, (gpointer) combo);
                        gtk_widget_set_name (GTK_WIDGET(combo), base_korr[k]);
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "есть");
                        gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "нет");
                        checkbutton = gtk_check_button_new_with_label (korr_name[k]);
                        //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
                        g_signal_connect (G_OBJECT (checkbutton), "toggled",
                                G_CALLBACK (state_callback), (gpointer) combo);
                        // Уберем все галки
                        g_signal_emit_by_name(checkbutton, "toggled", (gpointer) combo);
                        gtk_container_add (GTK_CONTAINER(hbox), checkbutton);
                        gtk_container_add (GTK_CONTAINER(hbox), combo);
                        gtk_table_attach_defaults (GTK_TABLE (table_in), hbox, 0+m, 1+m, 0+n, 1+n);
                        k++;
                    }
        }

    }


    g_printf("Array len = %d\n", array->len);

    for (i=0; i<array->len; i++)
    {
        //g_print("%d: %s\n" ,i, gtk_widget_get_name(g_ptr_array_index(array, i)));
    }
//****************************************************************************************
/*    for (i=0; i<array->len; i++)
    {
        gboolean val;
        gchar str_1;
        val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_ptr_array_index(array, 0)));
        if (val == TRUE)
        {
            g_ptr_array_add (arr2, (gpointer) g_ptr_array_index (array, i));
            //loop by k
            str = g_strconcat("SELECT MODEL FROM ", base_frame[k], NULL);
//            str_1 = func (arr2, "SELECT MODEL FROM" table[i]); 
//            //g_print ("%d\n", val);
        }
        
    }   */

/*    printf("----------------\n");
    for (i=0; i<arr2->len; i++)
    {
        //g_print("%s\n" ,GTK_OBJECT_TYPE_NAME (g_ptr_array_index(arr2, i)));
    }*/

    sch_button = gtk_button_new_with_label ("Поиск"); 
    gtk_widget_set_size_request (sch_button, 70, 35);
    clr_button = gtk_button_new_with_label ("Сброс"); 
    gtk_widget_set_size_request (clr_button, 70, 35);

    g_signal_connect (G_OBJECT (sch_button), "clicked",
            G_CALLBACK (sch_callback), (gpointer) array);//array 
    g_signal_connect (G_OBJECT (clr_button), "clicked",
            G_CALLBACK (clr_callback), (gpointer) array);


    for ( i=0; i<array->len; i++ )
        if(g_strcmp0(GTK_OBJECT_TYPE_NAME(g_ptr_array_index(array,i)), "GtkEntry") == 0)  
            g_signal_connect(G_OBJECT(g_ptr_array_index(array, i)), "activate",
                    G_CALLBACK(entry_enter), (gpointer) sch_button);


    /*checkbuttons**************************************************
     * hbox = gtk_hbox_new(FALSE, 0);
     *
     * for (i=0; i<6; i++)
     * {
     * checkbutton = gtk_check_button_new_with_label (sch_filter[i]);
     * gtk_container_add (GTK_CONTAINER (hbox), checkbutton);
     * }*/

    stores = g_ptr_array_new();
    sizes = g_array_new(FALSE, FALSE, sizeof(int));

    for (i=0; i<6; i++)
    {   
        if (i==0)
        {
            tempStore = gtk_tree_store_new(4,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING);


            g_ptr_array_add(stores, tempStore);
            size = 4;
            tableOut = setup_table_xu(tempStore, input_entry_name_table, size);
            

        }
        else if (i==1)
        {
            size = 17;
            tempStore = gtk_tree_store_new(size,
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
   //                 G_TYPE_STRING,  //5 new
                    G_TYPE_STRING);
            g_ptr_array_add(stores, tempStore);
            tableOut = setup_table_xu(tempStore, output_entry_name_table, size);
        }
        else if (i==2)
        {
            tempStore = gtk_tree_store_new(7,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING);
            g_ptr_array_add(stores, tempStore);
            size = 7;
            tableOut = setup_table_xu(tempStore, work_name_table, size);
        }
        else if (i==3)
        {
            size = 5;
            tempStore = gtk_tree_store_new(size,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING);
            g_ptr_array_add(stores, tempStore);
            tableOut = setup_table_xu(tempStore, defend1_name_table, size);
        }
        else if (i==4)
        {
            size = 3;
            tempStore = gtk_tree_store_new(size,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING);
            g_ptr_array_add(stores, tempStore);
            tableOut = setup_table_xu(tempStore, defend2_name_table, size);
        }
        else if (i==5)
        {
            size = 4;
            tempStore = gtk_tree_store_new(size,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING,
                    G_TYPE_STRING);
            g_ptr_array_add(stores, tempStore);
            tableOut = setup_table_xu(tempStore, korr_name_table, size);
        }

        clear_table(size, tempStore);
        g_array_append_val(sizes, size);

        scr_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scr_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scr_window), tableOut);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scr_window, gtk_label_new(frame_name[i]));
    }


    bbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX (bbox), clr_button, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX (bbox), sch_button, TRUE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_table_attach_defaults (GTK_TABLE (table), bbox, 2, 3, 13, 14);
    gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0, 3, 14, 15);
    
    return table;
}
