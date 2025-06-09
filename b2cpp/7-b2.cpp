/* 2452098 计算机 赵崇治 */
#include <iostream>
#include <cstring>
#include "cmd_console_tools.h"
#include "7-b2.h"
using namespace std;

/***************************************************************************
  函数名称：get_length
  功    能：计算包含单字节和双字节字符（如汉字）的字符串的可视宽度。
  返 回 值：字符串在控制台中占用的总列数。
  说    明：假设双字节字符的第一个字节作为有符号char时其值小于0。
***************************************************************************/
static int get_length(const char* str) {
    if (!str)
        return 0;
    int length = 0;
    for (int i = 0; str[i] != '\0';) {
        if (str[i] < 0) { // 检查是否为双字节字符
            length += 2;
            i += 2;
        }
        else {
            length += 1;
            i += 1;
        }
    }
    return length;
}

/***************************************************************************
  函数名称：print_line_truncated
  功    能：在控制台指定位置打印一行文本，并将其截断至指定的可视宽度。
  输入参数：x, y - 控制台的起始坐标（列，行）。
            text - 需要打印的字符串。
            max_width - 文本可以占用的最大列数。
            bg, fg - 背景色和前景色。
  说    明：确保在截断时不会将一个双字节字符拆分。
***************************************************************************/
static void print_line_truncated(int x, int y, const char* text, int max_width, int bg, int fg) {
    // 避免溢出
    char buffer[MAX_ITEM_LEN * 2];
    int buffer_idx = 0;
    int current_width = 0;

    // 构建截断后的字符串
    for (int i = 0; text[i] != '\0' && current_width < max_width;) {
        if (text[i] < 0) { // 双字节字符
            if (current_width + 2 <= max_width) {
                buffer[buffer_idx++] = text[i];
                buffer[buffer_idx++] = text[i + 1];
                current_width += 2;
                i += 2;
            }
            else {
                break; // 剩余空间不足以打印
            }
        }
        else { // 单字节字符
            if (current_width + 1 <= max_width) {
                buffer[buffer_idx++] = text[i];
                current_width += 1;
                i += 1;
            }
            else {
                break;
            }
        }
    }

    // 用空格填充剩余部分
    for (int i = 0; i < max_width - current_width; ++i) {
        buffer[buffer_idx++] = ' ';
    }
    buffer[buffer_idx] = '\0'; // 添加字符串结束符

    // 使用 cct_showstr 统一输出
    cct_showstr(x, y, buffer, bg, fg);
}

int pop_menu(const char menu[][MAX_ITEM_LEN], const struct PopMenu* para) {
    if (!menu || !menu[0] || strlen(menu[0]) == 0)
        return 0; // 空菜单直接返回

    int item_count = 0;
    while (menu[item_count] && strlen(menu[item_count]) > 0)
        item_count++;
    if (item_count == 0)
        return 0;

    cct_setcursor(CURSOR_INVISIBLE);
    cct_enable_mouse();
    int original_bg, original_fg;
    cct_getcolor(original_bg, original_fg);

    int title_width = get_length(para->title);
    int content_width = para->width; // 改个名字容易看
    if (content_width < title_width) {
        content_width = title_width;
    }
    if (content_width % 2 != 0) {
        content_width++; // 确保内容宽度为偶数，以便边框对齐
    }

    int item_height = 0;
    if (para->high < item_count)
        item_height = para->high;
    else
        item_height = item_count;

    int current_selection = 0;
    int top_visible_item = 0;
    int final_selection = 0;

    const int content_x = para->start_x + 2;
    const int right_border_x = content_x + content_width;

    // 绘制带标题的上边框
    int title_padding_left = (content_width - title_width) / 2;
    cct_showstr(para->start_x, para->start_y, "╔", para->bg_color, para->fg_color);
    cct_showstr(content_x, para->start_y, "═", para->bg_color, para->fg_color, content_width / 2);
    cct_showstr(content_x + title_padding_left, para->start_y, para->title, para->bg_color, para->fg_color);
    cct_showstr(right_border_x, para->start_y, "╗", para->bg_color, para->fg_color);

    // 绘制菜单项和侧边框
    for (int i = 0; i < item_height; ++i) {
        int y = para->start_y + 1 + i;
        cct_showstr(para->start_x, y, "║", para->bg_color, para->fg_color);

        int item_idx = top_visible_item + i;
        bool is_selected = (item_idx == current_selection);
        int bg = 0;
        int fg = 0;
        if (is_selected) {
            bg = para->fg_color;
            fg = para->bg_color; // 反色
        }
        else {
            bg = para->bg_color;
            fg = para->fg_color;
        }
        print_line_truncated(content_x, y, menu[item_idx], content_width, bg, fg);

        cct_showstr(right_border_x, y, "║", para->bg_color, para->fg_color);
    }

    // 绘制下边框
    int bottom_y = para->start_y + 1 + item_height;
    cct_showstr(para->start_x, bottom_y, "╚", para->bg_color, para->fg_color);
    cct_showstr(content_x, bottom_y, "═", para->bg_color, para->fg_color, content_width / 2);
    cct_showstr(right_border_x, bottom_y, "╝", para->bg_color, para->fg_color);
    bool is_running = true;
    while (is_running) {
        int mx, my, maction, keycode1, keycode2;
        int event = cct_read_keyboard_and_mouse(mx, my, maction, keycode1, keycode2);

        if (event == CCT_KEYBOARD_EVENT) {
            int prev_selection = current_selection;
            if (keycode1 == 224) { // 方向键
                if (keycode2 == KB_ARROW_UP && current_selection > 0)
                    current_selection--;
                else if (keycode2 == KB_ARROW_DOWN && current_selection < item_count - 1)
                    current_selection++;
            }
            else if (keycode1 == '\r') { // 回车键
                final_selection = current_selection + 1;
                is_running = false;
            }
            else if (keycode1 == 27) { // ESC键
                final_selection = 0;
                is_running = false;
            }

            if (is_running && prev_selection != current_selection) {
                // 如果选项移出可视区，则滚动并重绘整个菜单
                if (current_selection < top_visible_item || current_selection >= top_visible_item + item_height) { // 分别对应上滚出和下滚出，可见区域上闭下开
                    if (current_selection < top_visible_item) {
                        top_visible_item = current_selection;
                    }
                    else {
                        top_visible_item = current_selection - item_height + 1;
                    }
                    const int content_x = para->start_x + 2;
                    const int right_border_x = content_x + content_width;

                    // 绘制带标题的上边框
                    int title_padding_left = (content_width - title_width) / 2;
                    cct_showstr(para->start_x, para->start_y, "╔", para->bg_color, para->fg_color);
                    cct_showstr(content_x, para->start_y, "═", para->bg_color, para->fg_color, content_width / 2);
                    cct_showstr(content_x + title_padding_left, para->start_y, para->title, para->bg_color, para->fg_color);
                    cct_showstr(right_border_x, para->start_y, "╗", para->bg_color, para->fg_color);

                    // 绘制菜单项和侧边框
                    for (int i = 0; i < item_height; ++i) {
                        int y = para->start_y + 1 + i;
                        cct_showstr(para->start_x, y, "║", para->bg_color, para->fg_color);

                        int item_idx = top_visible_item + i;
                        bool is_selected = (item_idx == current_selection);
                        int bg = 0;
                        int fg = 0;
                        if (is_selected) {
                            bg = para->fg_color;
                            fg = para->bg_color; // 反色
                        }
                        else {
                            bg = para->bg_color;
                            fg = para->fg_color;
                        }
                        print_line_truncated(content_x, y, menu[item_idx], content_width, bg, fg);

                        cct_showstr(right_border_x, y, "║", para->bg_color, para->fg_color);
                    }

                    // 绘制下边框
                    int bottom_y = para->start_y + 1 + item_height;
                    cct_showstr(para->start_x, bottom_y, "╚", para->bg_color, para->fg_color);
                    cct_showstr(content_x, bottom_y, "═", para->bg_color, para->fg_color, content_width / 2);
                    cct_showstr(right_border_x, bottom_y, "╝", para->bg_color, para->fg_color);
                }
                else { // 只重绘变化的两个项目
                    if (prev_selection >= top_visible_item && prev_selection < top_visible_item + item_height) {
                        int y = para->start_y + 1 + (prev_selection - top_visible_item);
                        int bg = para->bg_color;
                        int fg = para->fg_color;
                        print_line_truncated(para->start_x + 2, y, menu[prev_selection], content_width, bg, fg);
                    }

                    if (current_selection >= top_visible_item && current_selection < top_visible_item + item_height) {
                        int y = para->start_y + 1 + (current_selection - top_visible_item);
                        int bg = para->fg_color;
                        int fg = para->bg_color; // 反色
                        print_line_truncated(para->start_x + 2, y, menu[current_selection], content_width, bg, fg);
                    }
                }
            }
        }
        else if (event == CCT_MOUSE_EVENT) {
            if (maction == MOUSE_RIGHT_BUTTON_CLICK) {
                final_selection = 0;
                is_running = false;
            }
            else if (maction == MOUSE_WHEEL_MOVED_UP && top_visible_item > 0) {
                top_visible_item--;
                const int content_x = para->start_x + 2;
                const int right_border_x = content_x + content_width;

                // 绘制带标题的上边框
                int title_padding_left = (content_width - title_width) / 2;
                cct_showstr(para->start_x, para->start_y, "╔", para->bg_color, para->fg_color);
                cct_showstr(content_x, para->start_y, "═", para->bg_color, para->fg_color, content_width / 2);
                cct_showstr(content_x + title_padding_left, para->start_y, para->title, para->bg_color, para->fg_color);
                cct_showstr(right_border_x, para->start_y, "╗", para->bg_color, para->fg_color);

                // 绘制菜单项和侧边框
                for (int i = 0; i < item_height; ++i) {
                    int y = para->start_y + 1 + i;
                    cct_showstr(para->start_x, y, "║", para->bg_color, para->fg_color);

                    int item_idx = top_visible_item + i;
                    bool is_selected = (item_idx == current_selection);
                    int bg = 0;
                    int fg = 0;
                    if (is_selected) {
                        bg = para->fg_color;
                        fg = para->bg_color; // 反色
                    }
                    else {
                        bg = para->bg_color;
                        fg = para->fg_color;
                    }
                    print_line_truncated(content_x, y, menu[item_idx], content_width, bg, fg);

                    cct_showstr(right_border_x, y, "║", para->bg_color, para->fg_color);
                }

                // 绘制下边框
                int bottom_y = para->start_y + 1 + item_height;
                cct_showstr(para->start_x, bottom_y, "╚", para->bg_color, para->fg_color);
                cct_showstr(content_x, bottom_y, "═", para->bg_color, para->fg_color, content_width / 2);
                cct_showstr(right_border_x, bottom_y, "╝", para->bg_color, para->fg_color);
            }
            else if (maction == MOUSE_WHEEL_MOVED_DOWN && top_visible_item + item_height < item_count) {
                top_visible_item++;
                const int content_x = para->start_x + 2;
                const int right_border_x = content_x + content_width;

                // 绘制带标题的上边框
                int title_padding_left = (content_width - title_width) / 2;
                cct_showstr(para->start_x, para->start_y, "╔", para->bg_color, para->fg_color);
                cct_showstr(content_x, para->start_y, "═", para->bg_color, para->fg_color, content_width / 2);
                cct_showstr(content_x + title_padding_left, para->start_y, para->title, para->bg_color, para->fg_color);
                cct_showstr(right_border_x, para->start_y, "╗", para->bg_color, para->fg_color);

                // 绘制菜单项和侧边框
                for (int i = 0; i < item_height; ++i) {
                    int y = para->start_y + 1 + i;
                    cct_showstr(para->start_x, y, "║", para->bg_color, para->fg_color);

                    int item_idx = top_visible_item + i;
                    bool is_selected = (item_idx == current_selection);
                    int bg = 0;
                    int fg = 0;
                    if (is_selected) {
                        bg = para->fg_color;
                        fg = para->bg_color; // 反色
                    }
                    else {
                        bg = para->bg_color;
                        fg = para->fg_color;
                    }
                    print_line_truncated(content_x, y, menu[item_idx], content_width, bg, fg);

                    cct_showstr(right_border_x, y, "║", para->bg_color, para->fg_color);
                }

                // 绘制下边框
                int bottom_y = para->start_y + 1 + item_height;
                cct_showstr(para->start_x, bottom_y, "╚", para->bg_color, para->fg_color);
                cct_showstr(content_x, bottom_y, "═", para->bg_color, para->fg_color, content_width / 2);
                cct_showstr(right_border_x, bottom_y, "╝", para->bg_color, para->fg_color);
            }

            // 检查鼠标是否在菜单项区域内
            if (my >= para->start_y + 1 && my < para->start_y + 1 + item_height &&
                mx >= para->start_x + 2 && mx < para->start_x + 2 + content_width) {
                int hovered_item = top_visible_item + (my - (para->start_y + 1));
                if (hovered_item != current_selection) {
                    if (current_selection >= top_visible_item && current_selection < top_visible_item + item_height) {
                        int y = para->start_y + 1 + (current_selection - top_visible_item);
                        int bg = para->bg_color;
                        int fg = para->fg_color;
                        print_line_truncated(para->start_x + 2, y, menu[current_selection], content_width, bg, fg);
                    }
                    current_selection = hovered_item;
                    if (current_selection >= top_visible_item && current_selection < top_visible_item + item_height) {
                        int y = para->start_y + 1 + (current_selection - top_visible_item);
                        int bg = para->fg_color;
                        int fg = para->bg_color; // 反色
                        print_line_truncated(para->start_x + 2, y, menu[current_selection], content_width, bg, fg);
                    }
                }
                if (maction == MOUSE_LEFT_BUTTON_CLICK) {
                    final_selection = current_selection + 1;
                    is_running = false;
                }
            }
        }
    }

    cct_disable_mouse();
    cct_setcursor(CURSOR_VISIBLE_NORMAL);
    cct_setcolor(original_bg, original_fg);

    return final_selection;
}