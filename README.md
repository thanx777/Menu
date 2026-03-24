# STM32 菜单系统使用指南

## 目录

1. [系统概述](#1-系统概述)
2. [文件结构](#2-文件结构)
3. [核心数据结构](#3-核心数据结构)
4. [快速入门](#4-快速入门)
5. [API函数详解](#5-api函数详解)
6. [菜单构建步骤](#6-菜单构建步骤)
7. [完整示例](#7-完整示例)
8. [常见问题](#8-常见问题)

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

## 3. 核心数据结构

### 3.1 菜单节点 (MENUITEM)

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

### 3.2 导航栈 (MENU_STACK)

```c
#define MENU_STACK_MAX_DEPTH  8    // 最大菜单深度

typedef struct {
    MENUITEM *items[MENU_STACK_MAX_DEPTH];
    uint8_t top;
} MENU_STACK;
```

### 3.3 按键定义

```c
#define MENU_KEY_CONFIRM    1    // 确认键
#define MENU_KEY_BACK       2    // 返回键
#define MENU_KEY_DOWN       3    // 下滑键
#define MENU_KEY_UP         4    // 上滑键
```

---

## 4. 快速入门

### 4.1 最简菜单示例

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

### 4.2 定义菜单节点

```c
MENUITEM menu_Main = {"主菜单", NULL, NULL, NULL, NULL, NULL};
MENUITEM menu_Setting = {"设置", NULL, NULL, NULL, NULL, NULL};
MENUITEM menu_LED = {"LED开关", Action_LED, NULL, NULL, NULL, NULL};
```

### 4.3 构建菜单关系

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

## 5. API函数详解

### 5.1 链表操作函数

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

### 5.2 栈操作函数

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

### 5.3 UI函数

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

## 6. 菜单构建步骤

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

## 7. 完整示例

### 菜单结构

```
主菜单
├── 系统设置
│   ├── LED控制      [执行功能]
│   └── 亮度调节      [执行功能]
├── 显示设置
│   └── 高级设置
│       ├── 对比度
│       └── 刷新率
└── 关于            [执行功能]
```

### 代码实现

参见 [Menu_Init.c](Menu_Init.c) 文件

---

## 8. 常见问题

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

### Q5: 如何实现参数调节菜单？

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
| `Menu_List_BindFirstLevel()` | 绑定一级菜单 | Menu_Init()中调用 |
| `Menu_Stack_Push()` | 压栈 | 进入子菜单时 |
| `Menu_Stack_Pop()` | 弹栈 | 返回上级时 |
