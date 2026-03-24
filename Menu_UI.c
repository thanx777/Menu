/**
 * @file    Menu_UI.c
 * @brief   菜单系统UI显示和按键处理
 * @details 实现菜单的OLED显示和按键响应逻辑
 *          - 显示函数：根据 g_pDisplayFirst 渲染菜单项
 *          - 按键处理：确认、返回、上滑、下滑
 * @author  [Your Name]
 * @date    2024
 */

#include "stm32f10x.h"
#include "Menu_Def.h"
#include "Menu_Func.h"
#include "OLED.h"
#include "OLED_Data.h"
#include <stdbool.h>

/*============================================================================*/
/*                          全局变量定义                                       */
/*============================================================================*/

MENU_STACK g_MenuStack;        // 菜单导航栈实例
MENUITEM* g_pDisplayFirst;     // 当前屏幕显示的首个菜单节点
uint8_t g_DisplayLines = 4;    // OLED显示行数，根据屏幕尺寸调整

/*============================================================================*/
/*                          内部函数声明                                       */
/*============================================================================*/

static void Menu_Key_Confirm(void);
static void Menu_Key_Back(void);
static void Menu_Key_Down(void);
static void Menu_Key_Up(void);

/*============================================================================*/
/*                          显示函数实现                                       */
/*============================================================================*/

/**
 * @brief  菜单显示核心函数
 * 
 * @note   显示逻辑：
 *         1. 以 g_pDisplayFirst 为首个显示项
 *         2. 依次显示后续 g_DisplayLines 个节点
 *         3. 首个节点（g_pDisplayFirst）作为当前选中项，可添加反显效果
 *         4. 循环链表天然支持循环滚动
 *         
 *         示例（4行屏幕，选中项反显）：
 *         ┌──────────────┐
 *         │ [菜单项1]    │ ← 选中项（反显）
 *         │  菜单项2     │
 *         │  菜单项3     │
 *         │  菜单项4     │
 *         └──────────────┘
 */
void Menu_UI_Display(void) {
    if (g_pDisplayFirst == NULL) {
        return;
    }
    
    OLED_Clear();
    
    MENUITEM* pTemp = g_pDisplayFirst;
    
    for (uint8_t i = 0; i < g_DisplayLines; i++) {
        if (i == 0) {
            OLED_Printf(0, i * 16, OLED_8X16, ">%s", pTemp->DisplayString);
        } else {
            OLED_Printf(0, i * 16, OLED_8X16, " %s", pTemp->DisplayString);
        }
        
        pTemp = pTemp->pNextMenu;
    }
    
    OLED_Update();
}

/*============================================================================*/
/*                          按键处理函数实现                                   */
/*============================================================================*/

/**
 * @brief  按键处理核心函数
 * @param  KeyNum 按键值
 *         - MENU_KEY_CONFIRM (1): 确认键
 *         - MENU_KEY_BACK    (2): 返回键
 *         - MENU_KEY_DOWN    (3): 下滑键
 *         - MENU_KEY_UP      (4): 上滑键
 * 
 * @note   使用示例：
 *         // 在按键扫描任务中调用
 *         uint8_t key = Key_Scan();
 *         if (key != 0) {
 *             Menu_Key_Process(key);
 *         }
 */
void Menu_Key_Process(uint16_t KeyNum) {
    switch(KeyNum) {
        case MENU_KEY_CONFIRM:
            Menu_Key_Confirm();
            break;
        case MENU_KEY_BACK:
            Menu_Key_Back();
            break;
        case MENU_KEY_DOWN:
            Menu_Key_Down();
            break;
        case MENU_KEY_UP:
            Menu_Key_Up();
            break;
        default:
            break;
    }
    Menu_UI_Display();
}

/**
 * @brief  确认键处理函数
 * 
 * @note   处理逻辑：
 *         1. 有子菜单 → 进入子菜单（压栈当前节点 + 更新显示起点为子菜单头）
 *         2. 无子菜单但有 action → 执行功能函数（不压栈）
 *         3. 无子菜单无 action → 无操作
 *         
 *         进入子菜单流程：
 *         ┌─────────┐      确认键      ┌─────────┐
 *         │ 父菜单  │ ──────────────→ │ 子菜单  │
 *         └─────────┘                  └─────────┘
 *              ↑                            │
 *              │        压栈记录路径         │
 *              └────────────────────────────┘
 *         
 *         重要：压栈的是当前选中的菜单项，而不是子菜单头节点
 *         这样返回时可以恢复到之前的选中位置
 */
static void Menu_Key_Confirm(void) {
    if (g_pDisplayFirst == NULL) {
        return;
    }

    if (g_pDisplayFirst->pChildMenu != NULL) {
        if (Menu_Stack_Push(&g_MenuStack, g_pDisplayFirst)) {
            g_pDisplayFirst = g_pDisplayFirst->pChildMenu;
        }
    }
    else if (g_pDisplayFirst->action != NULL) {
        g_pDisplayFirst->action();
    }
}

/**
 * @brief  返回键处理函数
 * 
 * @note   处理逻辑：
 *         1. 栈为空 → 无操作
 *         2. 栈中只有一个元素（一级菜单）→ 恢复到初始位置
 *         3. 栈中有多个元素 → 弹栈，恢复到父菜单的选中位置
 *         
 *         返回上级菜单流程：
 *         ┌─────────┐      返回键      ┌─────────┐
 *         │ 子菜单  │ ──────────────→ │ 父菜单  │
 *         └─────────┘                  └─────────┘
 *              │                            ↑
 *              │        弹栈恢复位置         │
 *              └────────────────────────────┘
 *         
 *         重要：栈中存储的是进入子菜单前的选中位置
 *         弹栈后直接恢复到该位置
 */
static void Menu_Key_Back(void) {
    if (Menu_Stack_IsEmpty(&g_MenuStack)) {
        return;
    }
    
    if (g_MenuStack.top == 1) {
        g_pDisplayFirst = g_MenuStack.items[0];
        return;
    }
    
    g_pDisplayFirst = Menu_Stack_Pop(&g_MenuStack);
}

/**
 * @brief  下滑键处理函数
 * 
 * @note   处理逻辑：
 *         将 g_pDisplayFirst 更新为其 pNextMenu
 *         循环链表自动处理边界，无需额外判断
 *         
 *         示例（4项菜单循环滚动）：
 *         显示: A → B → C → D → A → B → ...
 *         按下键: A → B, B → C, D → A
 */
static void Menu_Key_Down(void) {
    if (g_pDisplayFirst == NULL) {
        return;
    }
    g_pDisplayFirst = g_pDisplayFirst->pNextMenu;
}

/**
 * @brief  上滑键处理函数
 * 
 * @note   处理逻辑：
 *         将 g_pDisplayFirst 更新为其 pPrevMenu
 *         循环链表自动处理边界，无需额外判断
 *         
 *         示例（4项菜单循环滚动）：
 *         显示: A ← B ← C ← D ← A ← B ← ...
 *         按上键: B → A, A → D, C → B
 */
static void Menu_Key_Up(void) {
    if (g_pDisplayFirst == NULL) {
        return;
    }
    g_pDisplayFirst = g_pDisplayFirst->pPrevMenu;
}
