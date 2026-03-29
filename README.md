# STM32 菜单系统使用指南

## 目录

1. [系统概述](#1-系统概述)
2. [文件结构](#2-文件结构)
3. [平台移植](#3-平台移植)
4. [核心数据结构](#4-核心数据结构)
5. [快速入门](#5-快速入门)
6. [API函数详解](#6-api函数详解)
7. [菜单构建步骤](#7-菜单构建步骤)
8. [完整示例](#8-完整示例)
9. [常见问题](#9-常见问题)
10. [Action函数编写指南](#10-action函数编写指南)

---

## 1. 系统概述

本菜单系统专为STM32单片机设计，采用**循环双向链表 + 静态栈**架构，具有以下特点：

- **无限层级嵌套**：支持任意深度的菜单层级
- **循环滚动**：同级菜单项自动循环，无需边界判断
- **零动态内存**：全程静态分配，适合嵌入式环境
- **解耦设计**：菜单逻辑与显示驱动分离，易于移植

### 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                        菜单系统架构                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   ┌─────────────┐     ┌─────────────┐     ┌─────────────┐  │
│   │ Menu_Def.h  │────→│ Menu_Func.c │────→│  Menu_UI.c  │  │
│   │  数据结构   │     │  链表/栈操作 │     │  显示/按键  │  │
│   └─────────────┘     └─────────────┘     └─────────────┘  │
│          │                   │                   │          │
│          └───────────────────┴───────────────────┘          │
│                              │                              │
│                      ┌───────┴───────┐                      │
│                      │  Menu_Init.c  │                      │
│                      │   菜单初始化   │                      │
│                      └───────────────┘                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. 文件结构

| 文件 | 功能 | 必须修改 |
|------|------|----------|
| `Menu_Def.h` | 数据结构定义 | 否（仅调整配置） |
| `Menu_Func.h` | 功能函数声明 | 否 |
| `Menu_Func.c` | 链表/栈操作实现 | 否 |
| `Menu_UI.h` | UI接口声明 | 否 |
| `Menu_UI.c` | 显示/按键处理 | **是**（适配显示驱动） |
| `Menu_Init.h` | 初始化接口声明 | 否 |
| `Menu_Init.c` | 菜单结构定义 | **是**（定义您的菜单） |

---

## 3. 平台移植

本菜单系统支持多种开发平台，通过条件编译实现平台切换。

### 3.1 支持的平台

| 平台 | 宏定义 | 头文件 |
|------|--------|--------|
| STM32 标准库 | `MENU_USE_STD_LIB` | `stm32f10x.h` |
| STM32 HAL库 | `MENU_USE_HAL_LIB` | `stm32f1xx_hal.h` |
| 其他平台 | 两者都为0 | 自行添加 |

### 3.2 切换平台

在以下文件开头修改宏定义：
- `Menu_UI.c`
- `Menu_Func.c`
- `Menu_Init.c`

```c
/*============================================================================*/
/*                          平台头文件选择                                     */
/*============================================================================*/
/*
 * 使用说明：根据你的开发平台，选择对应的头文件
 * 
 * 方式1：标准库（STM32F10x）
 *   #define MENU_USE_STD_LIB    1
 *   #define MENU_USE_HAL_LIB    0
 * 
 * 方式2：HAL库（STM32F1xx）
 *   #define MENU_USE_STD_LIB    0
 *   #define MENU_USE_HAL_LIB    1
 * 
 * 方式3：其他平台（如ESP32、Arduino等）
 *   #define MENU_USE_STD_LIB    0
 *   #define MENU_USE_HAL_LIB    0
 *   然后在下方添加你的平台头文件
 */

#define MENU_USE_STD_LIB    1    /* 改为 0 禁用标准库 */
#define MENU_USE_HAL_LIB    0    /* 改为 1 启用HAL库 */

#if MENU_USE_STD_LIB
    #include "stm32f10x.h"
#elif MENU_USE_HAL_LIB
    #include "stm32f1xx_hal.h"
#else
    /* 其他平台头文件 */
#endif
```

### 3.3 移植到其他平台

1. **修改宏定义**
   ```c
   #define MENU_USE_STD_LIB    0
   #define MENU_USE_HAL_LIB    0
   ```

2. **添加平台头文件**
   ```c
   #else
       #include "esp_idf.h"    /* 例如：ESP-IDF */
       // 或
       #include "Arduino.h"    /* 例如：Arduino */
   #endif
   ```

3. **适配OLED驱动**
   - 修改 `OLED_W_SCL()` 和 `OLED_W_SDA()` 函数中的GPIO操作
   - 修改延时函数调用

### 3.4 OLED驱动适配

OLED驱动需要适配以下硬件相关函数：

| 函数 | 位置 | 说明 |
|------|------|------|
| `OLED_W_SCL()` | OLED.c | SCL引脚高低电平控制 |
| `OLED_W_SDA()` | OLED.c | SDA引脚高低电平控制 |
| `OLED_GPIO_Init()` | OLED.c | I2C引脚初始化 |
| `HAL_Delay()` | OLED.c | 延时函数 |

**示例：适配ESP32**
```c
void OLED_W_SCL(uint8_t BitValue)
{
    gpio_set_level(GPIO_NUM_18, BitValue);
}

void OLED_W_SDA(uint8_t BitValue)
{
    gpio_set_level(GPIO_NUM_19, BitValue);
}
```

---

## 4. 核心数据结构

### 4.1 菜单节点 (MENUITEM)

```c
typedef struct MENUITEM {
    char DisplayString[20];           // 菜单显示名称
    MENU_ACTION_FUNC action;          // 功能函数指针
    struct MENUITEM *pParentMenu;     // 父菜单指针
    struct MENUITEM *pChildMenu;      // 子菜单头节点
    struct MENUITEM *pPrevMenu;       // 前驱节点（同级）
    struct MENUITEM *pNextMenu;       // 后继节点（同级）
} MENUITEM;
```

### 4.2 导航栈 (MENU_STACK)

```c
#define MENU_STACK_MAX_DEPTH  8    // 最大菜单深度

typedef struct {
    MENUITEM *items[MENU_STACK_MAX_DEPTH];
    uint8_t top;
} MENU_STACK;
```

### 4.3 按键定义

```c
#define MENU_KEY_CONFIRM    1    // 确认键
#define MENU_KEY_BACK       2    // 返回键
#define MENU_KEY_DOWN       3    // 下滑键
#define MENU_KEY_UP         4    // 上滑键
```

---

## 5. 快速入门

### 5.1 最简菜单示例

```c
#include "Menu_Init.h"

int main(void) {
    OLED_Init();
    Menu_Init();
    
    while(1) {
        uint8_t key = Key_Scan();
        if (key != 0) {
            Menu_Key_Process(key);
        }
    }
}
```

### 5.2 定义菜单节点

```c
MENUITEM menu_Main = {"主菜单", NULL, NULL, NULL, NULL, NULL};
MENUITEM menu_Setting = {"设置", NULL, NULL, NULL, NULL, NULL};
MENUITEM menu_LED = {"LED开关", Action_LED, NULL, NULL, NULL, NULL};
```

### 5.3 构建菜单关系

```c
void Menu_Init(void) {
    Menu_Stack_Init(&g_MenuStack);
    
    Menu_List_BindFirstLevel(&menu_Main, &menu_Setting);
    Menu_List_BindChild(&menu_Main, &menu_LED);
    
    Menu_Stack_Push(&g_MenuStack, &menu_Main);
    g_pDisplayFirst = &menu_Main;
    
    Menu_UI_Display();
}
```

---

## 6. API函数详解

### 6.1 链表操作函数

#### Menu_List_BindChild - 绑定父子菜单

```c
bool Menu_List_BindChild(MENUITEM *pParent, MENUITEM *pChild);
```

| 参数 | 说明 |
|------|------|
| pParent | 父菜单节点指针 |
| pChild | 子菜单节点指针 |
| **返回值** | true=成功，false=失败 |

**使用示例：**
```c
Menu_List_BindChild(&menu_System, &menu_LED);
Menu_List_BindChild(&menu_System, &menu_Brightness);
```

**效果：**
```
系统设置
  ├── LED控制      ← 第一个子菜单
  └── 亮度调节      ← 第二个子菜单
```

---

#### Menu_List_BindFirstLevel - 绑定一级菜单

```c
bool Menu_List_BindFirstLevel(MENUITEM *pFirst, MENUITEM *pNew);
```

| 参数 | 说明 |
|------|------|
| pFirst | 一级菜单的首个节点 |
| pNew | 待添加的新节点 |
| **返回值** | true=成功，false=失败 |

**使用示例：**
```c
Menu_List_BindFirstLevel(&menu_System, &menu_Display);
Menu_List_BindFirstLevel(&menu_System, &menu_About);
```

**效果：**
```
系统设置 ↔ 显示设置 ↔ 关于 ↔ 系统设置（循环）
```

---

#### Menu_List_InitSingleFirstLevel - 初始化单个一级菜单

```c
bool Menu_List_InitSingleFirstLevel(MENUITEM *pMenu);
```

| 参数 | 说明 |
|------|------|
| pMenu | 一级菜单节点指针 |
| **返回值** | true=成功，false=失败 |

**使用场景：** 当一级菜单只有一个节点时使用

**使用示例：**
```c
/* 只有一个一级菜单的情况 */
static MENUITEM menu_Main = {"主菜单", NULL, NULL, NULL, NULL, NULL};

void Menu_Init(void) {
    Menu_Stack_Init(&g_MenuStack);
    
    /* 初始化单个一级菜单（形成自环） */
    Menu_List_InitSingleFirstLevel(&menu_Main);
    
    /* 绑定子菜单 */
    Menu_List_BindChild(&menu_Main, &menu_Sub1);
    Menu_List_BindChild(&menu_Main, &menu_Sub2);
    
    Menu_Stack_Push(&g_MenuStack, &menu_Main);
    g_pDisplayFirst = &menu_Main;
    
    Menu_UI_Display();
}
```

**效果：**
```
主菜单（自环）
  ├── 子菜单1
  └── 子菜单2
```

---

### 6.2 栈操作函数

#### Menu_Stack_Init - 初始化导航栈

```c
void Menu_Stack_Init(MENU_STACK *pStack);
```

**使用示例：**
```c
Menu_Stack_Init(&g_MenuStack);
```

---

#### Menu_Stack_Push - 压栈

```c
bool Menu_Stack_Push(MENU_STACK *pStack, MENUITEM *pMenu);
```

**使用示例：**
```c
Menu_Stack_Push(&g_MenuStack, &menu_System);
```

---

#### Menu_Stack_Pop - 弹栈

```c
MENUITEM* Menu_Stack_Pop(MENU_STACK *pStack);
```

**返回值：** 弹出的菜单节点指针，NULL表示栈空

---

#### Menu_Stack_GetTop - 获取栈顶

```c
MENUITEM* Menu_Stack_GetTop(MENU_STACK *pStack);
```

**返回值：** 栈顶菜单节点指针，不弹栈

---

### 6.3 UI函数

#### Menu_UI_Display - 刷新显示

```c
void Menu_UI_Display(void);
```

**说明：** 根据 `g_pDisplayFirst` 渲染当前菜单页

---

#### Menu_Key_Process - 按键处理

```c
void Menu_Key_Process(uint16_t KeyNum);
```

| 参数 | 说明 |
|------|------|
| KeyNum | 按键值（MENU_KEY_CONFIRM/BACK/DOWN/UP） |

**使用示例：**
```c
uint8_t key = Key_Scan();
if (key != 0) {
    Menu_Key_Process(key);
}
```

---

## 7. 菜单构建步骤

### 步骤1：定义菜单节点

```c
static MENUITEM menu_System = {
    "系统设置",    // 显示名称
    NULL,          // 功能函数（有子菜单则为NULL）
    NULL,          // 父菜单（初始化时自动绑定）
    NULL,          // 子菜单（初始化时绑定）
    NULL,          // 前驱（初始化时自动绑定）
    NULL           // 后继（初始化时自动绑定）
};
```

### 步骤2：定义功能函数（可选）

```c
void Action_LED_Toggle(void) {
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, 
        (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5)));
}

static MENUITEM menu_LED = {
    "LED开关",
    Action_LED_Toggle,  // 绑定功能函数
    NULL, NULL, NULL, NULL
};
```

### 步骤3：在Menu_Init中构建关系

```c
void Menu_Init(void) {
    // 1. 初始化栈
    Menu_Stack_Init(&g_MenuStack);
    
    // 2. 绑定一级菜单
    Menu_List_BindFirstLevel(&menu_System, &menu_Display);
    Menu_List_BindFirstLevel(&menu_System, &menu_About);
    
    // 3. 绑定子菜单
    Menu_List_BindChild(&menu_System, &menu_LED);
    Menu_List_BindChild(&menu_System, &menu_Brightness);
    Menu_List_BindChild(&menu_Display, &menu_Advanced);
    
    // 4. 设置初始显示
    Menu_Stack_Push(&g_MenuStack, &menu_System);
    g_pDisplayFirst = &menu_System;
    
    // 5. 首次渲染
    Menu_UI_Display();
}
```

### 步骤4：在main中调用

```c
int main(void) {
    OLED_Init();
    Menu_Init();
    
    while(1) {
        uint8_t key = Key_Scan();
        if (key != 0) {
            Menu_Key_Process(key);
        }
        Delay_ms(10);
    }
}
```

---

## 8. 完整示例

### 8.1 示例场景

假设我们要构建一个智能温控器的菜单系统，包含以下功能：

```
主菜单
├── 温度设置          [子菜单]
│   ├── 目标温度      [参数调节]
│   └── 温度校准      [参数调节]
├── 系统设置          [子菜单]
│   ├── LED控制       [开关功能]
│   ├── 蜂鸣器        [开关功能]
│   └── 恢复出厂      [执行功能]
└── 关于设备          [显示信息]
```

### 8.2 第一步：定义功能函数

在 `Menu_Init.c` 中定义各个功能函数：

```c
#include "stm32f10x.h"
#include "Menu_Def.h"
#include "Menu_Func.h"
#include "Menu_UI.h"

/* 全局变量 */
static uint8_t g_TargetTemp = 25;
static uint8_t g_Calibration = 0;
static uint8_t g_LEDSate = 0;
static uint8_t g_BuzzerState = 0;

/*---------------------------------- 功能函数 ----------------------------------*/

/**
 * @brief 目标温度调节
 * @note  这是一个参数调节型菜单项的典型实现
 */
void Action_TargetTemp(void) {
    OLED_Clear();
    
    while(1) {
        /* 显示当前温度 */
        OLED_Printf(0, 0, OLED_8X16, "目标温度");
        OLED_Printf(0, 24, OLED_8X16, "  %d C", g_TargetTemp);
        OLED_Printf(0, 48, OLED_8X16, "+  -  返回");
        OLED_Update();
        
        /* 按键处理 */
        uint8_t key = Key_Scan();
        if (key == MENU_KEY_UP) {
            if (g_TargetTemp < 35) g_TargetTemp++;
        }
        else if (key == MENU_KEY_DOWN) {
            if (g_TargetTemp > 10) g_TargetTemp--;
        }
        else if (key == MENU_KEY_BACK) {
            break;  /* 退出调节，返回菜单 */
        }
        Delay_ms(150);
    }
    
    /* 退出前清屏，让菜单系统重新渲染 */
    OLED_Clear();
}

/**
 * @brief 温度校准
 */
void Action_Calibration(void) {
    OLED_Clear();
    
    while(1) {
        OLED_Printf(0, 0, OLED_8X16, "温度校准");
        OLED_Printf(0, 24, OLED_8X16, "  %+d C", (int8_t)g_Calibration);
        OLED_Printf(0, 48, OLED_8X16, "+  -  返回");
        OLED_Update();
        
        uint8_t key = Key_Scan();
        if (key == MENU_KEY_UP) {
            if (g_Calibration < 10) g_Calibration++;
        }
        else if (key == MENU_KEY_DOWN) {
            if (g_Calibration > 0) g_Calibration--;
        }
        else if (key == MENU_KEY_BACK) {
            break;
        }
        Delay_ms(150);
    }
    
    OLED_Clear();
}

/**
 * @brief LED开关
 * @note  这是一个简单的开关型菜单项
 */
void Action_LED_Toggle(void) {
    g_LEDSate = !g_LEDSate;
    
    if (g_LEDSate) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
    
    /* 显示反馈 */
    OLED_Clear();
    OLED_Printf(0, 24, OLED_8X16, "LED: %s", g_LEDSate ? "ON" : "OFF");
    OLED_Update();
    Delay_ms(500);
}

/**
 * @brief 蜂鸣器开关
 */
void Action_Buzzer_Toggle(void) {
    g_BuzzerState = !g_BuzzerState;
    
    if (g_BuzzerState) {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    }
    
    OLED_Clear();
    OLED_Printf(0, 24, OLED_8X16, "Buzzer: %s", g_BuzzerState ? "ON" : "OFF");
    OLED_Update();
    Delay_ms(500);
}

/**
 * @brief 恢复出厂设置
 * @note  这是一个执行型菜单项，执行一次操作
 */
void Action_FactoryReset(void) {
    OLED_Clear();
    OLED_Printf(0, 0, OLED_8X16, "恢复出厂?");
    OLED_Printf(0, 24, OLED_8X16, "确认  取消");
    OLED_Update();
    
    while(1) {
        uint8_t key = Key_Scan();
        if (key == MENU_KEY_CONFIRM) {
            /* 执行恢复操作 */
            g_TargetTemp = 25;
            g_Calibration = 0;
            g_LEDSate = 0;
            g_BuzzerState = 0;
            GPIO_ResetBits(GPIOA, GPIO_Pin_5);
            GPIO_ResetBits(GPIOB, GPIO_Pin_8);
            
            OLED_Clear();
            OLED_Printf(0, 24, OLED_8X16, "已恢复!");
            OLED_Update();
            Delay_ms(1000);
            break;
        }
        else if (key == MENU_KEY_BACK) {
            break;
        }
    }
    
    OLED_Clear();
}

/**
 * @brief 显示关于信息
 */
void Action_About(void) {
    OLED_Clear();
    OLED_Printf(0, 0, OLED_8X16, "智能温控器");
    OLED_Printf(0, 16, OLED_8X16, "Ver: 1.0.0");
    OLED_Printf(0, 32, OLED_8X16, "2024-01-01");
    OLED_Printf(0, 48, OLED_8X16, "按返回键退出");
    OLED_Update();
    
    while(1) {
        if (Key_Scan() == MENU_KEY_BACK) {
            break;
        }
    }
    
    OLED_Clear();
}
```

### 8.3 第二步：定义菜单节点

```c
/*---------------------------------- 一级菜单 ----------------------------------*/

static MENUITEM menu_TempSetting = {
    "温度设置",       /* 显示名称 */
    NULL,             /* 无功能函数（有子菜单） */
    NULL,             /* 父菜单：一级菜单为NULL */
    NULL,             /* 子菜单：在Menu_Init中绑定 */
    NULL,             /* 前驱：自动绑定 */
    NULL              /* 后继：自动绑定 */
};

static MENUITEM menu_System = {
    "系统设置",
    NULL,
    NULL, NULL, NULL, NULL
};

static MENUITEM menu_About = {
    "关于设备",
    Action_About,     /* 直接绑定功能函数 */
    NULL, NULL, NULL, NULL
};

/*---------------------------------- 二级菜单 ----------------------------------*/

/* 温度设置的子菜单 */
static MENUITEM menu_TargetTemp = {
    "目标温度",
    Action_TargetTemp,  /* 绑定参数调节函数 */
    NULL, NULL, NULL, NULL
};

static MENUITEM menu_Calibration = {
    "温度校准",
    Action_Calibration,
    NULL, NULL, NULL, NULL
};

/* 系统设置的子菜单 */
static MENUITEM menu_LED = {
    "LED控制",
    Action_LED_Toggle,  /* 绑定开关函数 */
    NULL, NULL, NULL, NULL
};

static MENUITEM menu_Buzzer = {
    "蜂鸣器",
    Action_Buzzer_Toggle,
    NULL, NULL, NULL, NULL
};

static MENUITEM menu_Reset = {
    "恢复出厂",
    Action_FactoryReset,  /* 绑定执行函数 */
    NULL, NULL, NULL, NULL
};
```

### 8.4 第三步：构建菜单关系

```c
void Menu_Init(void) {
    /* 1. 初始化导航栈 */
    Menu_Stack_Init(&g_MenuStack);
    
    /* 2. 绑定一级菜单（形成循环链表） */
    /*    温度设置 ↔ 系统设置 ↔ 关于设备 ↔ 温度设置 */
    Menu_List_BindFirstLevel(&menu_TempSetting, &menu_System);
    Menu_List_BindFirstLevel(&menu_TempSetting, &menu_About);
    
    /* 3. 绑定二级菜单 */
    /*    温度设置
     *      ├── 目标温度
     *      └── 温度校准 */
    Menu_List_BindChild(&menu_TempSetting, &menu_TargetTemp);
    Menu_List_BindChild(&menu_TempSetting, &menu_Calibration);
    
    /*    系统设置
     *      ├── LED控制
     *      ├── 蜂鸣器
     *      └── 恢复出厂 */
    Menu_List_BindChild(&menu_System, &menu_LED);
    Menu_List_BindChild(&menu_System, &menu_Buzzer);
    Menu_List_BindChild(&menu_System, &menu_Reset);
    
    /* 4. 设置初始显示 */
    Menu_Stack_Push(&g_MenuStack, &menu_TempSetting);
    g_pDisplayFirst = &menu_TempSetting;
    
    /* 5. 首次渲染 */
    Menu_UI_Display();
}
```

### 8.5 第四步：主函数调用

```c
#include "Menu_Init.h"

int main(void) {
    /* 硬件初始化 */
    OLED_Init();
    Key_Init();
    GPIO_Init(GPIOA, GPIO_Pin_5, GPIO_Mode_Out_PP);  /* LED */
    GPIO_Init(GPIOB, GPIO_Pin_8, GPIO_Mode_Out_PP);  /* Buzzer */
    
    /* 菜单初始化 */
    Menu_Init();
    
    /* 主循环 */
    while(1) {
        uint8_t key = Key_Scan();
        if (key != 0) {
            Menu_Key_Process(key);
        }
        Delay_ms(10);
    }
}
```

### 8.6 菜单类型总结

| 菜单类型 | action字段 | pChildMenu字段 | 典型用途 |
|----------|------------|----------------|----------|
| **目录型** | NULL | 非NULL | 进入子菜单，如"温度设置" |
| **功能型** | 非NULL | NULL | 执行功能，如"LED控制" |
| **参数型** | 非NULL | NULL | 参数调节，如"目标温度" |
| **混合型** | 非NULL | 非NULL | 优先进入子菜单（不推荐） |

### 8.7 绑定关系图示

```
绑定函数调用顺序与结果：

Menu_List_BindFirstLevel(&menu_TempSetting, &menu_System);
Menu_List_BindFirstLevel(&menu_TempSetting, &menu_About);

    ┌─────────────┐
    │  温度设置   │ ← 一级菜单首节点
    └──────┬──────┘
           │ pNext
           ▼
    ┌─────────────┐
    │  系统设置   │
    └──────┬──────┘
           │ pNext
           ▼
    ┌─────────────┐
    │  关于设备   │
    └──────┬──────┘
           │ pNext (循环)
           ▼
    ┌─────────────┐
    │  温度设置   │ ← 回到首节点
    └─────────────┘


Menu_List_BindChild(&menu_TempSetting, &menu_TargetTemp);
Menu_List_BindChild(&menu_TempSetting, &menu_Calibration);

    温度设置 (父菜单)
        │
        │ pChildMenu
        ▼
    ┌─────────────┐
    │  目标温度   │ ← 子菜单首节点
    └──────┬──────┘
           │ pNext
           ▼
    ┌─────────────┐
    │  温度校准   │
    └──────┬──────┘
           │ pNext (循环)
           ▼
    ┌─────────────┐
    │  目标温度   │ ← 回到首节点
    └─────────────┘
```

---

## 9. 常见问题

### Q1: 如何修改显示行数？

修改 `Menu_UI.c` 中的 `g_DisplayLines` 变量：

```c
uint8_t g_DisplayLines = 4;  // 改为您的屏幕行数
```

### Q2: 如何适配其他显示屏？

修改 `Menu_UI.c` 中的 `Menu_UI_Display()` 函数，替换OLED相关函数为您的显示驱动。

### Q3: 如何增加菜单深度？

修改 `Menu_Def.h` 中的宏定义：

```c
#define MENU_STACK_MAX_DEPTH  16  // 增加到16级
```

### Q4: 按键如何映射？

在您的按键扫描函数中返回对应的宏值：

```c
uint8_t Key_Scan(void) {
    if (KEY_OK_PRESSED)    return MENU_KEY_CONFIRM;
    if (KEY_BACK_PRESSED)  return MENU_KEY_BACK;
    if (KEY_DOWN_PRESSED)  return MENU_KEY_DOWN;
    if (KEY_UP_PRESSED)    return MENU_KEY_UP;
    return 0;
}
```

### Q5: action函数中按返回键需要手动弹栈吗？

**不需要！** 这是本菜单系统的重要设计原则。

#### 栈操作规则

| 操作 | 栈变化 | 说明 |
|------|--------|------|
| 进入子菜单 | 压栈当前选中项 | 记录位置，返回时恢复 |
| 执行action | **不压栈** | action是叶子节点，无需记录 |
| 从子菜单返回 | 弹栈 | 恢复到进入前的选中位置 |
| 从action返回 | **不弹栈** | 因为根本没有压栈 |

#### 流程图解

```
┌─────────────────────────────────────────────────────────────────────┐
│                        栈操作流程                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  场景：从"系统设置"进入"LED控制"（action函数）                        │
│                                                                     │
│  ┌──────────────┐                                                   │
│  │   系统设置    │ ← 当前选中项                                      │
│  │   LED控制    │                                                   │
│  │   亮度调节    │                                                   │
│  └──────────────┘                                                   │
│         │                                                           │
│         │ 按确认键                                                   │
│         ▼                                                           │
│  ┌──────────────┐     栈状态：[系统设置]                             │
│  │  LED控制     │ ← 执行 Action_LED_Toggle()                        │
│  │  (action)    │     注意：栈没有变化！                              │
│  └──────────────┘                                                   │
│         │                                                           │
│         │ action函数内按返回键                                        │
│         │ (只是 break 退出循环)                                       │
│         ▼                                                           │
│  ┌──────────────┐     栈状态：[系统设置]                             │
│  │   系统设置    │ ← 自动回到菜单界面                                 │
│  │   LED控制    │     选中项仍然是"系统设置"                          │
│  │   亮度调节    │                                                   │
│  └──────────────┘                                                   │
│                                                                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  场景：从"系统设置"进入"高级设置"（子菜单）                           │
│                                                                     │
│  ┌──────────────┐                                                   │
│  │   系统设置    │ ← 当前选中项                                      │
│  │   高级设置    │                                                   │
│  └──────────────┘                                                   │
│         │                                                           │
│         │ 按确认键                                                   │
│         ▼                                                           │
│  ┌──────────────┐     栈状态：[高级设置]                             │
│  │   对比度      │ ← 进入子菜单，压栈"高级设置"                       │
│  │   刷新率      │     显示子菜单内容                                 │
│  └──────────────┘                                                   │
│         │                                                           │
│         │ 按返回键                                                   │
│         ▼                                                           │
│  ┌──────────────┐     栈状态：[] (弹栈后)                            │
│  │   系统设置    │ ← 弹栈恢复到"高级设置"                             │
│  │   高级设置    │ ← 选中项恢复到进入前的位置                          │
│  └──────────────┘                                                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

#### action函数的正确写法

```c
void Action_LED_Toggle(void) {
    static uint8_t state = 0;
    state = !state;
    
    if (state) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
    
    /* 简单的开关型：执行完直接返回 */
    /* 不需要处理返回键，菜单系统会自动刷新 */
}

void Action_Brightness_Set(void) {
    OLED_Clear();
    
    while(1) {
        /* 显示当前亮度 */
        OLED_Printf(0, 0, OLED_8X16, "亮度调节");
        OLED_Printf(0, 24, OLED_8X16, "  %d", brightness);
        OLED_Update();
        
        /* 自己处理按键 */
        uint8_t key = Key_Scan();
        if (key == MENU_KEY_UP)   brightness++;
        if (key == MENU_KEY_DOWN) brightness--;
        if (key == MENU_KEY_BACK) break;  /* 只是退出循环，不弹栈 */
        
        Delay_ms(100);
    }
    
    /* 退出循环后，菜单系统会自动刷新 */
    /* 不需要手动调用任何栈操作函数 */
}
```

#### 重要提醒

1. **action函数中不要调用** `Menu_Key_Process()`，这会导致递归
2. **action函数中不要调用** `Menu_Stack_Pop()`，因为栈中没有压入新元素
3. **action函数退出后**，菜单系统会自动调用 `Menu_UI_Display()` 刷新

---

## 10. Action函数编写指南

Action函数是菜单系统的核心扩展点，本节详细介绍各类Action函数的编写规范。

### 10.1 Action函数类型总览

| 类型 | 特点 | 典型应用 | 是否需要while循环 |
|------|------|----------|-------------------|
| **即时执行型** | 执行完立即返回 | LED开关、复位 | ❌ 不需要 |
| **参数调节型** | 需要持续交互 | 亮度、温度调节 | ✅ 需要 |
| **信息展示型** | 显示信息等待退出 | 关于、帮助 | ✅ 需要 |
| **确认对话框型** | 等待用户确认 | 恢复出厂、删除 | ✅ 需要 |

### 9.2 即时执行型Action

**特点**：执行一次操作后立即返回菜单

```c
/**
 * @brief LED开关 - 即时执行型示例
 * @note  执行完直接返回，无需while循环
 */
void Action_LED_Toggle(void) {
    static uint8_t state = 0;
    
    state = !state;
    
    if (state) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
    
    /* 函数结束，自动返回菜单 */
}
```

### 9.3 参数调节型Action

**特点**：需要while循环持续响应按键

```c
/**
 * @brief 温度调节 - 参数调节型示例
 * @note  必须用while循环持续处理按键
 */
void Action_TempAdjust(void) {
    static int8_t temp = 25;
    
    OLED_Clear();  /* 清屏准备新界面 */
    
    while(1) {
        /* ========== 1. 显示界面 ========== */
        OLED_Printf(0, 0, OLED_8X16, "目标温度");
        OLED_Printf(0, 24, OLED_8X16, "  %d C", temp);
        OLED_Printf(0, 48, OLED_8X16, "+  -  返回");
        OLED_Update();  /* 刷新显示 */
        
        /* ========== 2. 按键处理 ========== */
        uint8_t key = Key_Scan();
        
        switch(key) {
            case MENU_KEY_UP:
                if (temp < 35) temp++;
                break;
                
            case MENU_KEY_DOWN:
                if (temp > 10) temp--;
                break;
                
            case MENU_KEY_BACK:
                /* 退出循环，返回菜单 */
                goto exit;
        }
        
        Delay_ms(150);  /* 消抖延时 */
    }
    
exit:
    OLED_Clear();  /* 退出前清屏 */
    /* 函数结束，菜单系统自动刷新 */
}
```

### 10.4 信息展示型Action

**特点**：显示信息，等待用户按返回键退出

```c
/**
 * @brief 关于信息 - 信息展示型示例
 */
void Action_About(void) {
    OLED_Clear();
    
    /* 显示静态信息 */
    OLED_Printf(0, 0, OLED_8X16, "智能温控器");
    OLED_Printf(0, 16, OLED_8X16, "版本: 1.0.0");
    OLED_Printf(0, 32, OLED_8X16, "日期: 2024-01");
    OLED_Printf(0, 48, OLED_8X16, "按返回键退出");
    OLED_Update();
    
    /* 等待返回键 */
    while(1) {
        if (Key_Scan() == MENU_KEY_BACK) {
            break;
        }
    }
    
    OLED_Clear();
}
```

### 10.5 确认对话框型Action

**特点**：需要用户确认后才执行操作

```c
/**
 * @brief 恢复出厂设置 - 确认对话框型示例
 */
void Action_FactoryReset(void) {
    uint8_t selected = 0;  /* 0=取消, 1=确认 */
    
    OLED_Clear();
    
    while(1) {
        /* 显示确认对话框 */
        OLED_Printf(0, 0, OLED_8X16, "恢复出厂设置?");
        
        if (selected == 0) {
            OLED_Printf(0, 32, OLED_8X16, "[取消]  确认");
        } else {
            OLED_Printf(0, 32, OLED_8X16, " 取消  [确认]");
        }
        
        OLED_Update();
        
        /* 按键处理 */
        uint8_t key = Key_Scan();
        
        switch(key) {
            case MENU_KEY_UP:
            case MENU_KEY_DOWN:
                selected = !selected;  /* 切换选项 */
                break;
                
            case MENU_KEY_CONFIRM:
                if (selected == 1) {
                    /* 执行恢复操作 */
                    OLED_Clear();
                    OLED_Printf(0, 24, OLED_8X16, "正在恢复...");
                    OLED_Update();
                    
                    /* 这里执行实际的恢复逻辑 */
                    Delay_ms(1000);
                    
                    OLED_Clear();
                    OLED_Printf(0, 24, OLED_8X16, "恢复完成!");
                    OLED_Update();
                    Delay_ms(500);
                }
                goto exit;
                
            case MENU_KEY_BACK:
                goto exit;
        }
        
        Delay_ms(150);
    }
    
exit:
    OLED_Clear();
}
```

### 10.6 Action函数编写模板

#### 模板A：即时执行型

```c
void Action_XXX(void) {
    /* 1. 执行操作 */
    
    /* 2. 可选：显示反馈 */
    
    /* 自动返回菜单 */
}
```

#### 模板B：交互型（参数调节/信息展示/确认对话框）

```c
void Action_XXX(void) {
    /* 1. 初始化变量 */
    static int value = 0;
    
    /* 2. 清屏 */
    OLED_Clear();
    
    /* 3. 主循环 */
    while(1) {
        /* 3.1 显示界面 */
        OLED_Printf(...);
        OLED_Update();
        
        /* 3.2 按键处理 */
        uint8_t key = Key_Scan();
        
        switch(key) {
            case MENU_KEY_UP:
                /* 处理上键 */
                break;
                
            case MENU_KEY_DOWN:
                /* 处理下键 */
                break;
                
            case MENU_KEY_CONFIRM:
                /* 处理确认键（可选） */
                break;
                
            case MENU_KEY_BACK:
                /* 退出循环 */
                goto exit;
        }
        
        /* 3.3 延时消抖 */
        Delay_ms(150);
    }
    
exit:
    /* 4. 清理工作 */
    OLED_Clear();
    /* 自动返回菜单 */
}
```

### 10.7 常见错误与修正

| 错误写法 | 正确写法 | 原因 |
|----------|----------|------|
| `Menu_Key_Process(key)` | `Key_Scan()` + `switch` | 避免递归 |
| `Menu_Stack_Pop()` | 不调用栈操作 | action不压栈 |
| `Menu_UI_Display()` | 不调用 | 系统自动调用 |
| 忘记 `OLED_Clear()` | 进入/退出时清屏 | 避免残留 |
| 忘记 `OLED_Update()` | 显示后调用 | 刷新屏幕 |

### 10.8 完整示例：多级参数调节

```c
/**
 * @brief PID参数调节 - 复杂参数调节示例
 */
void Action_PID_Adjust(void) {
    typedef enum {
        PARAM_KP = 0,
        PARAM_KI,
        PARAM_KD,
        PARAM_COUNT
    } ParamType;
    
    static float params[PARAM_COUNT] = {1.0f, 0.1f, 0.5f};
    static const char* names[] = {"Kp", "Ki", "Kd"};
    uint8_t selected = PARAM_KP;
    
    OLED_Clear();
    
    while(1) {
        /* 显示所有参数 */
        OLED_Printf(0, 0, OLED_8X16, "PID参数调节");
        
        for (uint8_t i = 0; i < PARAM_COUNT; i++) {
            if (i == selected) {
                OLED_Printf(0, 16 + i * 16, OLED_8X16, ">%s=%.2f", 
                    names[i], params[i]);
            } else {
                OLED_Printf(0, 16 + i * 16, OLED_8X16, " %s=%.2f", 
                    names[i], params[i]);
            }
        }
        
        OLED_Update();
        
        uint8_t key = Key_Scan();
        
        switch(key) {
            case MENU_KEY_UP:
                if (selected > 0) selected--;
                break;
                
            case MENU_KEY_DOWN:
                if (selected < PARAM_COUNT - 1) selected++;
                break;
                
            case MENU_KEY_CONFIRM:
                /* 进入参数微调模式 */
                params[selected] += 0.1f;  /* 简化示例 */
                break;
                
            case MENU_KEY_BACK:
                goto exit;
        }
        
        Delay_ms(150);
    }
    
exit:
    OLED_Clear();
}
```

### Q6: 如何实现参数调节菜单？

在action函数中实现调节逻辑：

```c
void Action_Brightness_Set(void) {
    while(1) {
        uint8_t key = Key_Scan();
        if (key == MENU_KEY_UP)   brightness++;
        if (key == MENU_KEY_DOWN) brightness--;
        if (key == MENU_KEY_BACK) break;
        
        Display_Brightness(brightness);
        Delay_ms(100);
    }
}
```

---

## 附录：函数速查表

| 函数 | 用途 | 调用时机 |
|------|------|----------|
| `Menu_Init()` | 初始化菜单系统 | main()开头调用一次 |
| `Menu_Key_Process()` | 处理按键 | 按键扫描后调用 |
| `Menu_UI_Display()` | 刷新显示 | 需要刷新时调用 |
| `Menu_List_BindChild()` | 绑定子菜单 | Menu_Init()中调用 |
| `Menu_List_BindFirstLevel()` | 绑定多个一级菜单 | Menu_Init()中调用 |
| `Menu_List_InitSingleFirstLevel()` | 初始化单个一级菜单 | 一级菜单只有一个时 |
| `Menu_Stack_Push()` | 压栈 | 进入子菜单时 |
| `Menu_Stack_Pop()` | 弹栈 | 返回上级时 |
