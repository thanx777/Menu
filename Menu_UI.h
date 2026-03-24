/**
 * @file    Menu_UI.h
 * @brief   菜单系统UI接口声明
 * @details 声明菜单显示和按键处理的对外接口
 * @author  [Your Name]
 * @date    2024
 */

#ifndef __MENU_UI_H
#define __MENU_UI_H

#include <stdint.h>

/**
 * @brief  菜单显示函数
 * @note   根据 g_pDisplayFirst 渲染当前菜单页
 *         需要在按键操作后调用以刷新显示
 */
void Menu_UI_Display(void);

/**
 * @brief  按键处理函数
 * @param  KeyNum 按键值
 *         - MENU_KEY_CONFIRM (1): 确认键
 *         - MENU_KEY_BACK    (2): 返回键
 *         - MENU_KEY_DOWN    (3): 下滑键
 *         - MENU_KEY_UP      (4): 上滑键
 * @note   处理后会自动调用 Menu_UI_Display() 刷新显示
 */
void Menu_Key_Process(uint16_t KeyNum);

#endif /* __MENU_UI_H */
