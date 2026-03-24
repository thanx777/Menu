/**
 * @file    Menu_Init.c
 * @brief   菜单系统初始化实现
 * @details 定义菜单节点并构建菜单层级结构
 *          提供完整的多级菜单示例
 * @author  [Your Name]
 * @date    2024
 */

#include "stm32f10x.h"
#include "Menu_Def.h"
#include "Menu_Func.h"
#include "Menu_UI.h"

/*============================================================================*/
/*                          功能函数声明                                       */
/*============================================================================*/

void Action_LED_Toggle(void);
void Action_Brightness_Set(void);
void Action_About_Show(void);

/*============================================================================*/
/*                          一级菜单节点定义                                   */
/*============================================================================*/

/**
 * @brief 一级菜单：系统设置
 * @note  包含子菜单，无 action
 */
static MENUITEM menu_System = {
    "系统设置",           // 显示名称
    NULL,                 // 无功能函数（有子菜单）
    NULL,                 // 父菜单：一级菜单为NULL
    NULL,                 // 子菜单：在 Menu_Init() 中绑定
    NULL,                 // 前驱：在 Menu_Init() 中绑定
    NULL                  // 后继：在 Menu_Init() 中绑定
};

/**
 * @brief 一级菜单：显示设置
 */
static MENUITEM menu_Display = {
    "显示设置",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * @brief 一级菜单：关于
 * @note  无子菜单，绑定 action 功能函数
 */
static MENUITEM menu_About = {
    "关于",
    Action_About_Show,    // 直接绑定功能函数
    NULL,
    NULL,
    NULL,
    NULL
};

/*============================================================================*/
/*                          二级菜单节点定义                                   */
/*============================================================================*/

/**
 * @brief 二级菜单：LED控制（系统设置的子菜单）
 */
static MENUITEM menu_LED = {
    "LED控制",
    Action_LED_Toggle,    // 绑定功能函数
    NULL,                 // 父菜单：在 Menu_Init() 中绑定
    NULL,                 // 无子菜单
    NULL,
    NULL
};

/**
 * @brief 二级菜单：亮度调节（系统设置的子菜单）
 */
static MENUITEM menu_Brightness = {
    "亮度调节",
    Action_Brightness_Set,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * @brief 二级菜单：高级设置（显示设置的子菜单）
 * @note  包含三级子菜单
 */
static MENUITEM menu_Advanced = {
    "高级设置",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/*============================================================================*/
/*                          三级菜单节点定义                                   */
/*============================================================================*/

/**
 * @brief 三级菜单：对比度（高级设置的子菜单）
 */
static MENUITEM menu_Contrast = {
    "对比度",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * @brief 三级菜单：刷新率（高级设置的子菜单）
 */
static MENUITEM menu_RefreshRate = {
    "刷新率",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/*============================================================================*/
/*                          功能函数实现                                       */
/*============================================================================*/

/**
 * @brief LED开关功能
 */
void Action_LED_Toggle(void) {
    static uint8_t ledState = 0;
    ledState = !ledState;
    
    if (ledState) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
}

/**
 * @brief 亮度调节功能
 */
void Action_Brightness_Set(void) {
}

/**
 * @brief 显示关于信息
 */
void Action_About_Show(void) {
}

/*============================================================================*/
/*                          初始化函数实现                                     */
/*============================================================================*/

/**
 * @brief  菜单系统初始化
 * 
 * @note   菜单结构示意：
 *         
 *         ┌─────────────────────────────────────────────────────┐
 *         │                     一级菜单                        │
 *         │  ┌──────────┐  ┌──────────┐  ┌──────────┐          │
 *         │  │ 系统设置 │  │ 显示设置 │  │   关于   │          │
 *         │  └────┬─────┘  └────┬─────┘  └────┬─────┘          │
 *         │       │             │             │                 │
 *         │       ▼             ▼             ▼                 │
 *         │   二级菜单       二级菜单      执行action            │
 *         │  ┌────────┐     ┌────────┐                         │
 *         │  │LED控制 │     │高级设置│                         │
 *         │  └────────┘     └───┬────┘                         │
 *         │  ┌────────┐         │                              │
 *         │  │亮度调节│         ▼                              │
 *         │  └────────┘     三级菜单                           │
 *         │                 ┌────────┐                         │
 *         │                 │ 对比度 │                         │
 *         │                 └────────┘                         │
 *         │                 ┌────────┐                         │
 *         │                 │ 刷新率 │                         │
 *         │                 └────────┘                         │
 *         └─────────────────────────────────────────────────────┘
 */
void Menu_Init(void) {
    Menu_Stack_Init(&g_MenuStack);
    
    Menu_List_BindFirstLevel(&menu_System, &menu_Display);
    Menu_List_BindFirstLevel(&menu_System, &menu_About);
    
    Menu_List_BindChild(&menu_System, &menu_LED);
    Menu_List_BindChild(&menu_System, &menu_Brightness);
    
    Menu_List_BindChild(&menu_Display, &menu_Advanced);
    
    Menu_List_BindChild(&menu_Advanced, &menu_Contrast);
    Menu_List_BindChild(&menu_Advanced, &menu_RefreshRate);
    
    Menu_Stack_Push(&g_MenuStack, &menu_System);
    g_pDisplayFirst = &menu_System;
    
    Menu_UI_Display();
}
