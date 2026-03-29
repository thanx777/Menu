/**
 * @file    Menu_Func.c
 * @brief   菜单系统功能函数实现
 * @details 实现菜单链表操作和栈操作的核心函数
 *          - 循环双向链表的构建与维护
 *          - 静态栈的初始化、入栈、出栈等操作
 * @author  [Your Name]
 * @date    2024
 */

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

#define MENU_USE_STD_LIB    1
#define MENU_USE_HAL_LIB    0

#if MENU_USE_STD_LIB
    #include "stm32f10x.h"
#elif MENU_USE_HAL_LIB
    #include "stm32f1xx_hal.h"
#else
    /* 其他平台头文件 */
#endif

#include "Menu_Def.h"
#include "Menu_Func.h"
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================*/
/*                          链表操作函数实现                                   */
/*============================================================================*/

/**
 * @brief  绑定父子菜单节点（循环双向链表版）
 * @param  pParent 父菜单节点（不能为空）
 * @param  pChild  子菜单节点（不能为空，且未绑定其他父节点）
 * @return true=绑定成功，false=绑定失败
 * 
 * @note   实现原理：
 *         1. 子节点添加到父节点子链表的尾部
 *         2. 自动设置子节点的 pParentMenu 指向父节点
 *         3. 循环特性：尾节点的 pNext 指向头节点，头节点的 pPrev 指向尾节点
 *         
 *         示例：绑定 child1, child2, child3 到 parent
 *         第一次绑定 child1：
 *           parent.pChildMenu → child1
 *           child1.pPrev → child1, child1.pNext → child1（自环）
 *         
 *         第二次绑定 child2：
 *           child1.pPrev → child2, child1.pNext → child2
 *           child2.pPrev → child1, child2.pNext → child1
 *         
 *         第三次绑定 child3：
 *           child1 ↔ child2 ↔ child3 ↔ child1（循环）
 */
bool Menu_List_BindChild(MENUITEM *pParent, MENUITEM *pChild) {
    if (pParent == NULL || pChild == NULL) {
        return false;
    }
    
    if (pChild->pParentMenu != NULL && pChild->pParentMenu != pParent) {
        return false;
    }

    pChild->pParentMenu = pParent;

    if (pParent->pChildMenu == NULL) {
        pParent->pChildMenu = pChild;
        pChild->pPrevMenu = pChild;
        pChild->pNextMenu = pChild;
    } else {
        MENUITEM *pFirstChild = pParent->pChildMenu;
        MENUITEM *pLastChild = pFirstChild->pPrevMenu;

        pChild->pPrevMenu = pLastChild;
        pChild->pNextMenu = pFirstChild;
        pLastChild->pNextMenu = pChild;
        pFirstChild->pPrevMenu = pChild;
    }

    return true;
}

/**
 * @brief  初始化单个一级菜单节点（自环）
 * @param  pMenu 一级菜单节点指针
 * @return true=成功，false=失败
 * 
 * @note   当一级菜单只有一个节点时调用此函数
 *         使节点形成自环：pNextMenu 和 pPrevMenu 都指向自身
 */
bool Menu_List_InitSingleFirstLevel(MENUITEM *pMenu) {
    if (pMenu == NULL) {
        return false;
    }
    
    pMenu->pParentMenu = NULL;
    pMenu->pNextMenu = pMenu;
    pMenu->pPrevMenu = pMenu;
    
    return true;
}

/**
 * @brief  绑定一级菜单节点
 * @param  pFirst 一级菜单的首个节点
 * @param  pNew   待添加的新一级菜单节点
 * @return true=绑定成功，false=绑定失败
 * 
 * @note   实现原理：
 *         一级菜单没有父节点（pParentMenu = NULL）
 *         所有一级菜单形成独立的循环双向链表
 *         
 *         示例：绑定 menu1, menu2, menu3
 *         menu1 ↔ menu2 ↔ menu3 ↔ menu1（循环）
 *         
 *         注意：如果只有一个一级菜单，请使用 Menu_List_InitSingleFirstLevel()
 */
bool Menu_List_BindFirstLevel(MENUITEM *pFirst, MENUITEM *pNew) {
    if (pFirst == NULL || pNew == NULL) {
        return false;
    }
    
    if (pNew->pParentMenu != NULL) {
        return false;
    }
    
    pNew->pParentMenu = NULL;

    if (pFirst->pNextMenu == NULL || pFirst->pNextMenu == pFirst) {
        pFirst->pNextMenu = pNew;
        pFirst->pPrevMenu = pNew;
        pNew->pNextMenu = pFirst;
        pNew->pPrevMenu = pFirst;
    } else {
        MENUITEM *pLast = pFirst->pPrevMenu;

        pNew->pPrevMenu = pLast;
        pNew->pNextMenu = pFirst;
        pLast->pNextMenu = pNew;
        pFirst->pPrevMenu = pNew;
    }
    
    return true;
}

/*============================================================================*/
/*                          栈操作函数实现                                     */
/*============================================================================*/

/**
 * @brief  初始化菜单导航栈
 * @param  pStack 栈指针（不能为空）
 * 
 * @note   仅重置栈顶指针，数组内容无需清空
 *         静态分配的内存在程序运行期间一直存在
 */
void Menu_Stack_Init(MENU_STACK *pStack) {
    if (pStack == NULL) {
        return;
    }
    pStack->top = 0;
}

/**
 * @brief  菜单节点压栈
 * @param  pStack 栈指针
 * @param  pMenu  菜单节点指针
 * @return true=成功，false=失败（栈满或参数无效）
 * 
 * @note   进入子菜单时调用此函数
 *         将当前菜单节点压入栈中，记录导航路径
 */
bool Menu_Stack_Push(MENU_STACK *pStack, MENUITEM *pMenu) {
    if (pStack == NULL || pMenu == NULL) {
        return false;
    }
    
    if (pStack->top >= MENU_STACK_MAX_DEPTH) {
        return false;
    }

    pStack->items[pStack->top++] = pMenu;
    return true;
}

/**
 * @brief  菜单节点弹栈
 * @param  pStack 栈指针
 * @return 弹出的菜单节点指针，NULL表示栈空或参数无效
 * 
 * @note   返回上级菜单时调用此函数
 *         弹出栈顶节点，返回上一级菜单
 */
MENUITEM* Menu_Stack_Pop(MENU_STACK *pStack) {
    if (pStack == NULL || pStack->top == 0) {
        return NULL;
    }

    MENUITEM* pItem = pStack->items[--pStack->top];
    pStack->items[pStack->top] = NULL;
    return pItem;
}

/**
 * @brief  获取栈顶菜单节点（不弹栈）
 * @param  pStack 栈指针
 * @return 栈顶菜单节点指针，NULL表示栈空或参数无效
 * 
 * @note   查看当前所在菜单层，不改变栈状态
 */
MENUITEM* Menu_Stack_GetTop(MENU_STACK *pStack) {
    if (pStack == NULL || pStack->top == 0) {
        return NULL;
    }
    return pStack->items[pStack->top - 1];
}

/**
 * @brief  清空导航栈
 * @param  pStack 栈指针
 * 
 * @note   重置栈状态，清除所有导航记录
 */
void Menu_Stack_Clear(MENU_STACK *pStack) {
    if (pStack == NULL) {
        return;
    }
    
    for (uint8_t i = 0; i < pStack->top; i++) {
        pStack->items[i] = NULL;
    }
    pStack->top = 0;
}

/**
 * @brief  判断栈是否为空
 * @param  pStack 栈指针
 * @return true=空，false=非空
 */
bool Menu_Stack_IsEmpty(MENU_STACK *pStack) {
    if (pStack == NULL) {
        return true;
    }
    return (pStack->top == 0);
}

/**
 * @brief  判断栈是否已满
 * @param  pStack 栈指针
 * @return true=满，false=未满
 */
bool Menu_Stack_IsFull(MENU_STACK *pStack) {
    if (pStack == NULL) {
        return true;
    }
    return (pStack->top >= MENU_STACK_MAX_DEPTH);
}
