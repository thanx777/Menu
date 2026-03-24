/**
 * @file    Menu_Def.h
 * @brief   菜单系统数据结构定义
 * @details 定义菜单节点结构体、导航栈结构体、按键宏定义等核心数据结构
 *          采用循环双向链表实现菜单项管理，静态栈实现导航路径追踪
 * @author  [Your Name]
 * @date    2024
 */

#ifndef __MENU_DEF_H
#define __MENU_DEF_H

#include <stdint.h>
#include <stdlib.h>

/*============================================================================*/
/*                          类型定义                                           */
/*============================================================================*/

/**
 * @brief 菜单执行函数指针类型
 * @note  无参数、无返回值的回调函数，用于菜单项绑定功能函数
 *        例如：调节亮度、设置参数、执行操作等
 */
typedef void (*MENU_ACTION_FUNC)(void);

/**
 * @brief 菜单节点结构体
 * @note  采用循环双向链表结构，支持无限层级菜单嵌套
 *        - 同级菜单通过 pPrevMenu/pNextMenu 形成循环链表
 *        - 父子菜单通过 pParentMenu/pChildMenu 建立层级关系
 */
typedef struct MENUITEM {
    char DisplayString[20];           // 菜单显示名称，最大20字符
    MENU_ACTION_FUNC action;          // 功能函数指针，NULL表示无功能（子菜单项）
    struct MENUITEM *pParentMenu;     // 父菜单指针，一级菜单为NULL
    struct MENUITEM *pChildMenu;      // 子菜单头节点指针，无子菜单时为NULL
    struct MENUITEM *pPrevMenu;       // 前驱节点指针（同级循环链表）
    struct MENUITEM *pNextMenu;       // 后继节点指针（同级循环链表）
} MENUITEM;

/*============================================================================*/
/*                          栈配置                                             */
/*============================================================================*/

/**
 * @brief 菜单栈最大深度
 * @note  定义菜单最大嵌套层级，根据实际需求调整
 *        例如：深度8表示最多支持8级菜单嵌套
 */
#define MENU_STACK_MAX_DEPTH    8

/**
 * @brief 菜单导航栈结构体
 * @note  采用静态数组实现，避免动态内存分配
 *        - 进入子菜单时压栈（Push）
 *        - 返回上级菜单时弹栈（Pop）
 *        - 栈顶元素为当前所在菜单层
 */
typedef struct {
    MENUITEM *items[MENU_STACK_MAX_DEPTH];  // 栈存储数组，存放菜单节点指针
    uint8_t top;                            // 栈顶指针，0=空栈
} MENU_STACK;

/*============================================================================*/
/*                          按键定义                                           */
/*============================================================================*/

/**
 * @brief 按键值定义
 * @note  与 Menu_Key_Process() 函数配合使用
 *        用户需在按键扫描模块中映射实际按键到这些值
 */
#define MENU_KEY_CONFIRM    1    // 确认键：进入子菜单或执行功能函数
#define MENU_KEY_BACK       2    // 返回键：返回上级菜单
#define MENU_KEY_DOWN       3    // 下滑键：向下滚动菜单项
#define MENU_KEY_UP         4    // 上滑键：向上滚动菜单项

/*============================================================================*/
/*                          全局变量声明                                       */
/*============================================================================*/

/**
 * @brief 菜单导航栈（全局实例）
 * @note  在 Menu_UI.c 中定义，记录菜单访问路径
 */
extern MENU_STACK g_MenuStack;

/**
 * @brief 当前屏幕显示的首个菜单节点
 * @note  在 Menu_UI.c 中定义，作为显示起点
 *        - 按下/上键时更新为 next/prev 节点
 *        - 进入子菜单时更新为子菜单头节点
 */
extern MENUITEM* g_pDisplayFirst;

/**
 * @brief OLED显示行数
 * @note  根据实际屏幕尺寸调整，默认4行
 */
extern uint8_t g_DisplayLines;

#endif /* __MENU_DEF_H */
