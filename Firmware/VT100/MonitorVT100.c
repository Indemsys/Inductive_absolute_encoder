#include "App.h"

#define CNTLQ      0x11
#define CNTLS      0x13
#define DEL        0x7F
#define BACKSPACE  0x08
//#define CR         0x0D  конфликт имен с регистрами периферии
#define LF         0x0A
#define ESC        0x1B

#define COL        80   /* Maximum column size       */

#define EDSTLEN    20


static void Do_watchdog_test(uint8_t keycode);
static void Do_Reset(uint8_t keycode);

extern const T_VT100_Menu MENU_MAIN;
extern const T_VT100_Menu MENU_PARAMETERS;
extern const T_VT100_Menu MENU_SPEC;

#ifdef MDC01_APP
extern const T_VT100_Menu MENU_DMC01;
#endif

static int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b);

static void Do_serial_transmitter_test(uint8_t keycode);

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      Пункты имеющие свое подменю распологаются на следующем уровне вложенности
      Их функция подменяет в главном цикле обработчик нажатий по умолчанию и должна
      отдавать управление периодически в главный цикл

      Пункты не имеющие функции просто переходят на следующий уровень подменю

      Пункты не имеющие подменю полностью захватывают управление и на следующий уровень не переходят

      Пункты без подменю и функции означают возврат на предыдущий уровень
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//-------------------------------------------------------------------------------------
const T_VT100_Menu_item MENU_MAIN_ITEMS[] =
{
//  { '1', Do_Params_editor, (void *)&MENU_PARAMETERS },
#if APP_PLATF == ANSIN

  { '2', Do_LDC1101_diagnostic, 0},

#elif APP_PLATF == ANSMA

  { '2', Do_TLI5012_diagnostic, 0},

#else

#error  Must be selected platform

#endif
  { '3', Do_FlashMemoryDiagnostic, 0},
  { '4', Do_serial_transmitter_test, 0},
  { '6', 0, (void *)&MENU_SPEC },
  { '7', Do_Reset, 0  },
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_MAIN             =
{
  "ANSIN v1.0 Controller",
  "\033[5C MAIN MENU \r\n"
//  "\033[5C <1> - Adjustable parameters and settings\r\n"
#if APP_PLATF == ANSIN
  "\033[5C <2> - LDC11010 diagnostic\r\n"
#elif APP_PLATF == ANSMA
  "\033[5C <2> - TLI5012 diagnostic\r\n"
#else
#error  Must be selected platform
#endif
  "\033[5C <3> - Flash memory diagnostic\r\n"
  "\033[5C <4> - Serial transmitter test\r\n"
  "\033[5C <6> - Special menu\r\n"
  "\033[5C <7> - Reset\r\n",
  MENU_MAIN_ITEMS,
};

//-------------------------------------------------------------------------------------
const T_VT100_Menu      MENU_PARAMETERS       =
{
  "",
  "",
  0
};

const T_VT100_Menu_item MENU_SPEC_ITEMS[] =
{


  { '8', Do_watchdog_test, 0 },
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_SPEC  =
{
  "MONITOR Ver.160722",
  "\033[5C SPECIAL MENU \r\n"
  "\033[5C <8> - Watchdog test\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_SPEC_ITEMS,
};


const char              *_days_abbrev[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char              *_months_abbrev[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*-------------------------------------------------------------------------------------------------------------
   Процедура ожидания корректной последовательности данных для подтверждения входа с терминала
   Используется для фильтации шумов на длинном кабеле
-------------------------------------------------------------------------------------------------------------*/
const uint8_t             entry_str[] = "12345678";

static void Entry_check(void)
{
  uint32_t indx;
  uint8_t  b;
  T_monitor_cbl *pvt100_cb;
  OS_ERR             err;

  pvt100_cb = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);

  pvt100_cb->_printf("Press this sequence to enter - %s:\r\n", entry_str);
  indx = 0;
  while (1)
  {
    if (pvt100_cb->_wait_char(&b, 200) == RESULT_OK)
    {
      if (b != 0)
      {
        if (b == entry_str[indx])
        {
          indx++;
          if (indx == (sizeof(entry_str)- 1))
          {
            return;
          }
        }
        else
        {
          indx = 0;
        }
      }
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------
   Задача монитора
-------------------------------------------------------------------------------------------------------------*/
void Task_VT100(void *handle)
{
  uint8_t    b;
  OS_ERR     err;


  T_monitor_cbl *mcbl;
  T_monitor_driver *drv;

  // Выделим память для управляющей структуры задачи
  mcbl = (T_monitor_cbl *)_mem_alloc_zero(sizeof(T_monitor_cbl));
  if (mcbl == NULL) return;
  OSTaskRegSet(NULL, 0, (OS_REG)mcbl,&err);

  // Определим указатели функций ввода-вывода
  drv = (T_monitor_driver *)handle;
  mcbl->pdrv = drv;
  mcbl->_printf    = drv->_printf;
  mcbl->_wait_char = drv->_wait_char;
  mcbl->_send_buf  = drv->_send_buf;

  if (drv->_init(&mcbl->pdrvcbl, drv) != RESULT_OK) return; // Выходим из задачи если не удалось инициализировать драйвер.


//  for (;;)
//  {
//    mcbl->_send_buf(test_buf,64*3);
//    _time_delay(2);
//    _time_delay(2);
//  }




  // Очищаем экран
  mcbl->_printf(VT100_CLEAR_AND_HOME);


  //Entry_check();
  Goto_main_menu();

  do
  {
    if (mcbl->_wait_char(&b, 200) == RESULT_OK)
    {
      if (b != 0)
      {
        if ((b == 0x1B)) //&& (mcbl->Monitor_func != Edit_func))
        {
          mcbl->_printf(VT100_CLEAR_AND_HOME);
          Entry_check();
          Goto_main_menu();
        }
        else
        {
          if (mcbl->Monitor_func) mcbl->Monitor_func(b);  // Обработчик нажатий главного цикла
        }
      }
    }


    if (mcbl->menu_trace[mcbl->menu_nesting] == &MENU_MAIN)
    {
      VT100_set_cursor_pos(24, 0);
      mcbl->_printf("Firmware compile time: %s", __DATE__" "__TIME__);
    }
  }while (1);

}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Set_monitor_func(void (*func)(unsigned char))
{
  T_monitor_cbl *mcbl;
  OS_ERR             err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  mcbl->Monitor_func = func;
}



/*-------------------------------------------------------------------------------------------------------------
  Вывести на экран текущее меню
  1. Вывод заголовока меню
  2. Вывод содержания меню
-------------------------------------------------------------------------------------------------------------*/
void Display_menu(void)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  mcbl->_printf(VT100_CLEAR_AND_HOME);

  if (mcbl->menu_trace[mcbl->menu_nesting] == 0) return;

  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header, 1, Find_str_center((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_body, 3, 0);
  mcbl->_printf("\r\n");
  mcbl->_printf(DASH_LINE);

}
/*-------------------------------------------------------------------------------------------------------------
  Поиск в текущем меню пункта вызываемого передаваемым кодом
  Параметры:
    b - код команды вазывающей пункт меню
  Возвращает:
    Указатель на соответствующий пункт в текущем меню
-------------------------------------------------------------------------------------------------------------*/
int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b)
{
  int16_t i;
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  if (isalpha(b) != 0) b = toupper(b);

  i = 0;
  do
  {
    *item = (T_VT100_Menu_item *)mcbl->menu_trace[mcbl->menu_nesting]->menu_items + i;
    if ((*item)->but == b) return (1);
    if ((*item)->but == 0) break;
    i++;
  }while (1);

  return (0);
}


/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
static int Get_string(char *lp, int n)
{
  int  cnt = 0;
  char c;
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  do
  {
    if (mcbl->_wait_char((unsigned char *)&c, 200) == RESULT_OK)
    {
      switch (c)
      {
      case CNTLQ:                          /* ignore Control S/Q             */
      case CNTLS:
        break;

      case BACKSPACE:
      case DEL:
        if (cnt == 0)
        {
          break;
        }
        cnt--;                             /* decrement count                */
        lp--;                              /* and line VOID*               */
        /* echo backspace                 */
        mcbl->_printf("\x008 \x008");
        break;
      case ESC:
        *lp = 0;                           /* ESC - stop editing line        */
        return (RESULT_ERROR);
      default:
        mcbl->_printf("*");
        *lp = c;                           /* echo and store character       */
        lp++;                              /* increment line VOID*         */
        cnt++;                             /* and count                      */
        break;
      }
    }
  }while (cnt < n - 1 && c != 0x0d);      /* check limit and line feed      */
  *lp = 0;                                 /* mark end of string             */
  return (RESULT_OK);
}

/*-------------------------------------------------------------------------------------------------------------
  Ввод кода для доступа к специальному меню
-------------------------------------------------------------------------------------------------------------*/
int Enter_special_code(void)
{
  char str[32];
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  if (mcbl->g_access_to_spec_menu != 0)
  {
    return RESULT_OK;
  }
  mcbl->_printf(VT100_CLEAR_AND_HOME"Enter access code>");
  if (Get_string(str, 31) == RESULT_OK)
  {
    if (strcmp(str, "4321\r") == 0)
    {
      mcbl->g_access_to_spec_menu = 1;
      return RESULT_OK;
    }
  }

  return RESULT_ERROR;
}
/*-------------------------------------------------------------------------------------------------------------
  Поиск пункта меню по коду вызова (в текущем меню)
  и выполнение соответствующей ему функции
  Параметры:
    b - код команды вазывающей пункт меню

-------------------------------------------------------------------------------------------------------------*/
void Execute_menu_func(uint8_t b)
{
  T_VT100_Menu_item *menu_item;
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  if (Lookup_menu_item(&menu_item, b) != 0)
  {
    // Нашли соответствующий пункт меню
    if (menu_item->psubmenu != 0)
    {
      // Если присутствует субменю, то вывести его

      if ((T_VT100_Menu *)menu_item->psubmenu == &MENU_SPEC)
      {
        if (Enter_special_code() != RESULT_OK)
        {
          Display_menu();
          return;
        }
      }

      mcbl->menu_nesting++;
      mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)menu_item->psubmenu;

      Display_menu();
      // Если есть функция у пункта меню, то передать ей обработчик нажатий в главном цикле и выполнить функцию.
      if (menu_item->func != 0)
      {
        mcbl->Monitor_func = (T_menu_func)(menu_item->func); // Установить обработчик нажатий главного цикла на функцию из пункта меню
        menu_item->func(0);                // Выполнить саму функцию меню
      }
    }
    else
    {
      if (menu_item->func == 0)
      {
        // Если нет ни субменю ни функции, значит это пункт возврата в предыдущее меню
        // Управление остается в главном цикле у обработчика по умолчанию
        Return_to_prev_menu();
        Display_menu();
      }
      else
      {
        // Если у пункта нет своего меню, то перейти очистить экран и перейти к выполению  функции выбранного пункта
        mcbl->_printf(VT100_CLEAR_AND_HOME);
        menu_item->func(0);
        // Управление возвращается в главный цикл обработчику по умолчанию
        Display_menu();
      }
    }

  }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Goto_main_menu(void)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  mcbl->menu_nesting = 0;
  mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)&MENU_MAIN;
  Display_menu();
  mcbl->Monitor_func = Execute_menu_func; // Назначение функции
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Return_to_prev_menu(void)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  if (mcbl->menu_nesting > 0)
  {
    mcbl->menu_trace[mcbl->menu_nesting] = 0;
    mcbl->menu_nesting--;
  }
  mcbl->Monitor_func = Execute_menu_func; // Назначение функции
}



/*-------------------------------------------------------------------------------------------------------------
  Очистка экрана монитора
-------------------------------------------------------------------------------------------------------------*/
void VT100_clr_screen(void)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);

  mcbl->_printf(VT100_CLEAR_AND_HOME);
}

/*-------------------------------------------------------------------------------------------------------------
     Установка курсора в заданную позицию
     Счет начинается с 0
-------------------------------------------------------------------------------------------------------------*/
void VT100_set_cursor_pos(uint8_t row, uint8_t col)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);

  mcbl->_printf("\033[%.2d;%.2dH", row, col);
}

/*-------------------------------------------------------------------------------------------------------------
     Вывод строки в заданную позицию
-------------------------------------------------------------------------------------------------------------*/
void VT100_send_str_to_pos(uint8_t *str, uint8_t row, uint8_t col)
{
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);

  mcbl->_printf("\033[%.2d;%.2dH", row, col);
  mcbl->_send_buf(str, strlen((char *)str));
}

/*-------------------------------------------------------------------------------------------------------------
    Находим позицию начала строки для расположения ее по центру экрана
-------------------------------------------------------------------------------------------------------------*/
uint8_t Find_str_center(uint8_t *str)
{
  int16_t l = 0;
  while (*(str + l) != 0) l++; // Находим длину строки
  return (COLCOUNT - l) / 2;
}



/*-------------------------------------------------------------------------------------------------------------
  Прием строки
-------------------------------------------------------------------------------------------------------------*/
int32_t Mon_input_line(char *buf, int buf_len, int row, char *instr)
{

  int   i;
  uint8_t b;
  int   res;
  uint8_t bs_seq[] = { 0x08, 0x020, 0x08, 0 };
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  i = 0;
  VT100_set_cursor_pos(row, 0);
  mcbl->_printf(VT100_CLL_FM_CRSR);
  mcbl->_printf(">");

  if (instr != 0)
  {
    i = strlen(instr);
    if (i < buf_len)
    {
      mcbl->_printf(instr);
    }
    else i = 0;
  }

  do
  {
    if (mcbl->_wait_char(&b, 20000) != RESULT_OK)
    {
      res = RESULT_ERROR;
      goto exit_;
    };

    if (b == 0x1B)
    {
      res = RESULT_ERROR;
      goto exit_;
    }
    if (b == 0x08)
    {
      if (i > 0)
      {
        i--;
        mcbl->_send_buf(bs_seq, sizeof(bs_seq));
      }
    }
    else if (b != 0x0D && b != 0x0A && b != 0)
    {
      mcbl->_send_buf(&b, 1);
      buf[i] = b;           /* String[i] value set to alpha */
      i++;
      if (i >= buf_len)
      {
        res = RESULT_ERROR;
        goto exit_;
      };
    }
  }while ((b != 0x0D) && (i < COL));

  res = RESULT_OK;
  buf[i] = 0;                     /* End of string set to NUL */
exit_:

  VT100_set_cursor_pos(row, 0);
  mcbl->_printf(VT100_CLL_FM_CRSR);

  return (res);
}

/*------------------------------------------------------------------------------
 Вывод дампа области памяти


 \param addr       - выводимый начальный адрес дампа
 \param buf        - указатель на память
 \param buf_len    - количество байт
 \param sym_in_str - количество выводимых байт в строке дампа

 \return int32_t
 ------------------------------------------------------------------------------*/
void Mon_print_dump(uint32_t addr, void *buf, uint32_t buf_len, uint8_t sym_in_str)
{

  uint32_t   i;
  uint32_t   scnt;
  uint8_t    *pbuf;
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);


  pbuf = (uint8_t *)buf;
  scnt = 0;
  for (i = 0; i < buf_len; i++)
  {
    if (scnt == 0)
    {
      mcbl->_printf("%08X: ", addr);
    }

    mcbl->_printf("%02X ", pbuf[i]);

    addr++;
    scnt++;
    if (scnt >= sym_in_str)
    {
      scnt = 0;
      mcbl->_printf("\r\n");
    }
  }

  if (scnt != 0)
  {
    mcbl->_printf("\r\n");
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t Edit_integer_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv)
{
  char   str[32];
  char   buf[32];
  uint32_t tmpv;
  sprintf(str, "%d",*value);
  if (Mon_input_line(buf, 31, row, str) == RESULT_OK)
  {
    if (sscanf(buf, "%d",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
      return RESULT_OK;
    }
  }
  return RESULT_ERROR;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t Edit_integer_hex_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv)
{
  char   str[32];
  char   buf[32];
  uint32_t tmpv;
  sprintf(str, "%08X",*value);
  if (Mon_input_line(buf, 31, row, str) == RESULT_OK)
  {
    if (sscanf(buf, "%x",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
      return RESULT_OK;
    }
  }
  return RESULT_ERROR;
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Edit_float_val(uint32_t row, float *value, float minv, float maxv)
{
  char   str[32];
  char   buf[32];
  float tmpv;
  sprintf(str, "%f",*value);
  if (Mon_input_line(buf, 31, row, str) == RESULT_OK)
  {
    if (sscanf(buf, "%f",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
uint8_t BCD2ToBYTE(uint8_t value)
{
  uint32_t tmp;
  tmp =((value & 0xF0)>> 4) * 10;
  return(uint8_t)(tmp +(value & 0x0F));
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_watchdog_test(uint8_t keycode)
{
  uint8_t  b;
  T_monitor_cbl *mcbl;
  OS_ERR         err;
  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);

  mcbl->_printf(VT100_CLEAR_AND_HOME"Misc. test.\n\r");
  mcbl->_printf("Press 'R' to exit. \n\r");


// Тестирование работы watchdog
  {
    unsigned int i;
    __disable_interrupt();
    for (i = 0; i < 1000000000ul; i++)
    {
      __no_operation();
    }
    __enable_interrupt();
  }


  do
  {
    if (mcbl->_wait_char(&b, 200) == RESULT_OK)
    {
      switch (b)
      {
      case 'R':
      case 'r':
        return;
      }
    }
  }while (1);
}

uint32_t g_serial_transmitter_start;
uint32_t g_serial_transmitter_blsz = 4096;
uint32_t g_serial_transmitter_dtsz = 16777216;
uint32_t g_serial_transmitter_counter;
uint32_t g_serial_transmitter_repeat_counter;
/*-----------------------------------------------------------------------------------------------------
  Тестирование последовательного канала связи с PC
  Тест в режиме передатчика запускается по событию изменения внешней переменной
  В этом режиме непрерывно передаются инкрементирующиеся значения счетчика типа uint32_tР 
  
  Выход из функции только по окончанию счета  
-----------------------------------------------------------------------------------------------------*/
static void Do_serial_transmitter_test(uint8_t keycode)
{
  T_monitor_cbl *mcbl;

  uint32_t *block;
  uint32_t  i;
  uint32_t  dt_cnt;
  OS_ERR    err;

  mcbl = (T_monitor_cbl *)OSTaskRegGet(NULL, 0,&err);
  mcbl->_printf(VT100_CLEAR_AND_HOME"Serial transmitter test\n\r");
  mcbl->_printf(VT100_CLEAR_AND_HOME"Wait for signal...\n\r");

  g_serial_transmitter_start = 0;
  while (g_serial_transmitter_start == 0)
  {
    _time_delay(10);
  }

  g_serial_transmitter_blsz =(g_serial_transmitter_blsz / 4) * 4; // Приводим к размеру кратному 4
  block = malloc(g_serial_transmitter_blsz);

  g_serial_transmitter_counter = 0;
  g_serial_transmitter_repeat_counter = 0;
  do
  {
    // Заполняем блок
    dt_cnt = 0;
    for (i=0; i < g_serial_transmitter_blsz / 4; i++)
    {
      block[i] = g_serial_transmitter_counter;
      dt_cnt = dt_cnt + 4;

      g_serial_transmitter_counter = g_serial_transmitter_counter + 4;

      if (g_serial_transmitter_counter >= g_serial_transmitter_dtsz) break;
    }


    for (i=0; i < 20; i++)
    {
      if (mcbl->_send_buf(block,dt_cnt) == RESULT_OK) break;
      _time_delay(1);
      g_serial_transmitter_repeat_counter++;
    }
    if (i == 20)
    {
      break;
    }

  }while (g_serial_transmitter_counter < g_serial_transmitter_dtsz);

  g_serial_transmitter_start = 0;
}


/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
static void Do_Reset(uint8_t keycode)
{
  Reset_system();
}
