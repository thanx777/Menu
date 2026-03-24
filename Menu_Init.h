/**
 * @file    Menu_Init.h
 * @brief   菜单系统初始化接口
 * @details 声明菜单初始化函数，用于构建菜单结构
 * @author  [Your Name]
 * @date    2024
 */

#ifndef __MENU_INIT_H
#define __MENU_INIT_H

/**
 * @brief  菜单系统初始化函数
 * @note   在 main() 中调用，完成以下工作：
 *         1. 初始化导航栈
 *         2. 构建菜单层级结构
 *         3. 设置初始显示节点
 *         4. 首次渲染菜单
 */
void Menu_Init(void);

#endif /* __MENU_INIT_H */
