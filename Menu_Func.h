/**
 * @file    Menu_Func.h
 * @brief   菜单系统功能函数声明
 * @details 声明菜单链表操作函数和栈操作函数
 *          - 链表操作：菜单节点绑定、层级关系建立
 *          - 栈操作：导航路径管理（入栈、出栈、查询等）
 * @author  [Your Name]
 * @date    2024
 */

#ifndef __MENU_FUNC_H
#define __MENU_FUNC_H

#include <stdbool.h>
#include "Menu_Def.h"

/*============================================================================*/
/*                          链表操作函数                                       */
/*============================================================================*/

/**
 * @brief  绑定父子菜单节点
 * @param  pParent 父菜单节点指针
 * @param  pChild  子菜单节点指针
 * @return true=绑定成功，false=绑定失败
 * @note   使用示例：
 *         MENUITEM parent = {"父菜单", NULL, NULL, NULL, NULL, NULL};
 *         MENUITEM child = {"子菜单", NULL, NULL, NULL, NULL, NULL};
 *         Menu_List_BindChild(&parent, &child);
 *         
 *         绑定后：
 *         - child.pParentMenu 指向 parent
 *         - parent.pChildMenu 指向 child
 *         - 子节点自动形成循环双向链表
 */
bool Menu_List_BindChild(MENUITEM *pParent, MENUITEM *pChild);

/**
 * @brief  绑定一级菜单节点
 * @param  pFirst 一级菜单的首个节点
 * @param  pNew   待添加的新一级菜单节点
 * @return true=绑定成功，false=绑定失败
 * @note   使用示例：
 *         MENUITEM menu1 = {"菜单1", NULL, NULL, NULL, NULL, NULL};
 *         MENUITEM menu2 = {"菜单2", NULL, NULL, NULL, NULL, NULL};
 *         Menu_List_BindFirstLevel(&menu1, &menu2);
 *         
 *         绑定后：menu1 ↔ menu2 形成循环双向链表
 */
bool Menu_List_BindFirstLevel(MENUITEM *pFirst, MENUITEM *pNew);

/*============================================================================*/
/*                          栈操作函数                                         */
/*============================================================================*/

/**
 * @brief  初始化菜单导航栈
 * @param  pStack 栈指针
 * @note   使用示例：
 *         Menu_Stack_Init(&g_MenuStack);
 */
void Menu_Stack_Init(MENU_STACK *pStack);

/**
 * @brief  菜单节点压栈（进入子菜单时调用）
 * @param  pStack 栈指针
 * @param  pMenu  菜单节点指针
 * @return true=成功，false=失败（栈满或参数无效）
 * @note   使用示例：
 *         Menu_Stack_Push(&g_MenuStack, &subMenu);
 */
bool Menu_Stack_Push(MENU_STACK *pStack, MENUITEM *pMenu);

/**
 * @brief  菜单节点弹栈（返回上级菜单时调用）
 * @param  pStack 栈指针
 * @return 弹出的菜单节点指针，NULL表示栈空或参数无效
 * @note   使用示例：
 *         MENUITEM* pParent = Menu_Stack_Pop(&g_MenuStack);
 */
MENUITEM* Menu_Stack_Pop(MENU_STACK *pStack);

/**
 * @brief  获取栈顶菜单节点
 * @param  pStack 栈指针
 * @return 栈顶菜单节点指针，NULL表示栈空或参数无效
 * @note   使用示例：
 *         MENUITEM* pCurrent = Menu_Stack_GetTop(&g_MenuStack);
 */
MENUITEM* Menu_Stack_GetTop(MENU_STACK *pStack);

/**
 * @brief  清空导航栈
 * @param  pStack 栈指针
 * @note   使用示例：
 *         Menu_Stack_Clear(&g_MenuStack);
 */
void Menu_Stack_Clear(MENU_STACK *pStack);

/**
 * @brief  判断栈是否为空
 * @param  pStack 栈指针
 * @return true=空，false=非空
 */
bool Menu_Stack_IsEmpty(MENU_STACK *pStack);

/**
 * @brief  判断栈是否已满
 * @param  pStack 栈指针
 * @return true=满，false=未满
 */
bool Menu_Stack_IsFull(MENU_STACK *pStack);

#endif /* __MENU_FUNC_H */
