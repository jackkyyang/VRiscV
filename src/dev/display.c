/*
MIT License

Copyright (c) 2024 jackkyyang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
虚拟显示设备
根据CPU指令读取frame buffer内容，并输出到（显示器）窗口中
基于GTK+ 3开发，编译命令：
    gcc xxx.c -o xxx `pkg-config --cflags --libs gtk+-3.0`
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "display.h"

struct window_size_t
{
    gint x;
    gint y;
};

// 控件
static GtkWidget *window;//定义window控件
//创建文本框
// TextView 指针
static GtkWidget* view;

// 创建Entry
static GtkWidget* entry;

// 垂直布局
static GtkWidget* vbox;

static GtkWidget* scrowin_text;
static GtkWidget* scrowin_command;

//文本缓冲区的起始和结束位置
static GtkTextIter start, display_end, input_start,input_end;

static GtkTextMark* display_end_mark;

// Textbuffer 指针
static GtkTextBuffer *textbuffer;


// 自动滚动到最底层
static void auto_scroll(){
    GtkAdjustment* vadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrowin_text));
    // page_size 是显示窗口的尺寸（V,H）
    gdouble page_size = gtk_adjustment_get_page_size(GTK_ADJUSTMENT(vadjust));
    // upper是滚动条可滚动的最大的值
    gdouble upper = gtk_adjustment_get_upper(GTK_ADJUSTMENT(vadjust));

    // g_print("page_size: %f, ",page_size);
    // g_print("upper: %f\n",upper);
    // 将窗口自动滚动到最低位置
    gtk_adjustment_set_value(GTK_ADJUSTMENT(vadjust),(upper-page_size));

}

// 将字符串输出到屏幕并换行
static void display(const char* output_data,gint len)
{
    /*获取缓冲区的起始位置和结束位置*/
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(textbuffer),&display_end,display_end_mark);
    // 在mark结尾的位置插入新的文本
    gtk_text_buffer_insert(textbuffer,&display_end,output_data,len);

    //先将窗口向下滚动
    auto_scroll();

    // 插入换行符
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER(textbuffer),&display_end,display_end_mark);
    gtk_text_buffer_insert(textbuffer,&display_end,"\n",-1);

    return;

}

static void enter_callback(GtkWidget *widget,gpointer data)
{

    const gchar* command;
    guint16 command_len = 0;

    command = gtk_entry_get_text(GTK_ENTRY(entry));

    command_len = gtk_entry_get_text_length(GTK_ENTRY(entry));

    display(command,(gint)command_len);

    // 清空Entry内容
    GtkEntryBuffer* buf = gtk_entry_get_buffer(GTK_ENTRY(entry));
    gtk_entry_buffer_delete_text(GTK_ENTRY_BUFFER(buf),0,-1);

    return;

}

static int time_cnt = 0;
static gboolean do_timer( gpointer* null)
{
    gchar buf[64];

    time_cnt +=1;

    int len = sprintf(buf,"%d",time_cnt);

    display(buf,(gint)len);

    return TRUE;//尽量返回TRUE
}

void* screen_init(void* arg)
{

    struct window_size_t win_size = {1024,800};

    char* title = "Virtual Screen";

    //GTK组件
    gtk_init(0,NULL);

    //---------------------------------------------
    //主窗口
    //---------------------------------------------
    //创建窗口控件,设置为最上层主窗口
    //GTK_WINDOW_TOPLEVEL:  有边框
    //GTK_WINDOW_POPUP:     无边框
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // 设定窗口名
    gtk_window_set_title(GTK_WINDOW(window), title);

    // 设定窗口的大小
    gtk_window_set_default_size(GTK_WINDOW(window), win_size.x, win_size.y);

    // 设定窗口是否可以变化大小, 默认为TRUE
    // gtk_window_set_resizable (GTK_WINDOW(window),FALSE);

    // 获取当前窗口大小
    gtk_window_get_size (GTK_WINDOW(window),&win_size.x,&win_size.y);

    // 设置窗口边界
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);

    //---------------------------------------------
    // 设置布局
    //---------------------------------------------

    // 创建滚动窗口
    scrowin_text = gtk_scrolled_window_new  (NULL,NULL);
    scrowin_command = gtk_scrolled_window_new  (NULL,NULL);

    // 设置滚动条显示模式
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrowin_text),GTK_POLICY_AUTOMATIC,GTK_POLICY_ALWAYS);
    // gtk_scrolled_window_get_propagate_natural_width (GTK_SCROLLED_WINDOW(scrowin_text));
    // gtk_scrolled_window_set_max_content_width (GTK_SCROLLED_WINDOW(scrowin_text),20);


    entry= gtk_entry_new ();
    view = gtk_text_view_new();
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    gtk_container_add (GTK_CONTAINER(scrowin_text),view);
    gtk_container_add (GTK_CONTAINER(scrowin_command),entry);

    gtk_box_pack_start(GTK_BOX(vbox), scrowin_text, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), scrowin_command, TRUE, FALSE, 5);
    gtk_box_set_homogeneous(GTK_BOX(vbox),FALSE);
    // 将文本框放到窗口中
    gtk_container_add(GTK_CONTAINER(window),vbox);

    //---------------------------------------------
    // 文本框
    //---------------------------------------------
    //基于文本视图创建文本缓冲区
    textbuffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    // g_signal_connect(G_OBJECT(textbuffer),"mark_set", G_CALLBACK(text_callback),NULL);

    // 设置文本框为不可编辑
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);

    //设置自动换行
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view),GTK_WRAP_WORD_CHAR);

    // 设置名为bold的tag，将文本weight设置为粗体
    gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(textbuffer), "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    // 设置名为lock的tag，将文本设置为不可编辑
    gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(textbuffer), "lock", "editable", FALSE, NULL);


    /*获取文本缓冲区的起始地址和结束地址*/
    // 向开头位置插入文本，如果len==-1，则文本必须有终止符，且文本会被全部插入
    // gtk_text_buffer_get_bounds(textbuffer,&start,&display_end);
    // gtk_text_buffer_get_bounds(textbuffer,&start,&display_end);

    // 删除textbuffer中当前的内容，然后插入新的内容，如果len = -1, 则新的文本必须有终止符
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(textbuffer),"VRiscV Display initialization!\n",-1);
    gtk_text_buffer_get_bounds(textbuffer,&start,&display_end);
    // 将上述文本变为粗体
    gtk_text_buffer_apply_tag_by_name (GTK_TEXT_BUFFER(textbuffer),"bold",&start,&display_end);

    // 将display_end的位置绑定到display_end_mark上
    // 并且将mark定位到Root之前
    display_end_mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(textbuffer),"display_end", &display_end,FALSE);

    //---------------------------------------------
    // 命令行
    //---------------------------------------------
    gtk_entry_set_max_length(GTK_ENTRY(entry),0);
    gtk_entry_set_placeholder_text (GTK_ENTRY(entry),"command:");


    //-------------------------------------------
    // 事件绑定
    //-------------------------------------------
    // 在窗口被销毁时，退出 gtk_main 主循环
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    // 在输入Enter的时候，调用该函数
    g_signal_connect (entry, "activate", G_CALLBACK (enter_callback), NULL);

    // 绑定定时器
    guint timer = g_timeout_add(1000, (GSourceFunc)do_timer, NULL);

    // 所有空间默认隐藏，设置显示窗口和其中所有控件
    gtk_widget_show_all(window);

    // 主事件循环
    gtk_main();

    //删除定时器
    g_source_remove(timer);

}