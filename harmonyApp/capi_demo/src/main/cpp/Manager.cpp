//
// Created on 2025/9/28.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include "Manager.h"
#include <arkui/native_interface.h>
#include <arkui/native_animate.h>
// 引入 ArkUI 节点接口
#include <arkui/native_node.h>
// 引入 ArkUI 类型定义
#include <arkui/native_type.h>
// 引入 ArkUI 与 NAPI 的桥接接口
#include <arkui/native_node_napi.h>
// 引入 ArkUI 渲染接口
#include <arkui/native_render.h>

#include <hilog/log.h>
// 以下为鸿蒙 Drawing 渲染相关头文件
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_shader_effect.h>
#include <native_drawing/drawing_brush.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_pen.h>
#include <native_drawing/drawing_rect.h>
#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_canvas.h>

#include <cstdint>

namespace NativeXComponentSample {
#define TEST_TEXT_NUMBER 20
namespace {}
// 定义全局节点缓冲数组
ArkUI_NodeHandle gTextBuf[TEST_TEXT_NUMBER];


Manager Manager::manager_;

Manager::~Manager() {
    // 遍历缓存的 XComponent 指针并释放
    for (auto iter = nativeXComponentMap_.begin(); iter != nativeXComponentMap_.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
    // 清空映射避免悬挂引用
    nativeXComponentMap_.clear();

    // 遍历容器缓存并释放实例
    for (auto iter = containerMap_.begin(); iter != containerMap_.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
    // 清空容器映射
    containerMap_.clear();
}

// 演示如何创建并使用 RenderNode
// 该函数创建一个包含 RenderNode 的节点树，并为 RenderNode 设置属性和动画
ArkUI_NodeHandle render_node_add_event_demo(ArkUI_NativeNodeAPI_1 *nodeAPI, ArkUI_ContextHandle context) {
    // 创建列布局根节点
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    // 设置根节点宽度属性
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = {valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    // 设置根节点高度属性
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = {valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    // 设置根节点背景颜色
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    // 创建自定义节点用于承载 RenderNode
    auto custom_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    // 设置自定义节点宽度
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = {valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(custom_arkui_node, NODE_WIDTH, &itemWidth2);
    // 设置自定义节点高度
    ArkUI_NumberValue valueHeight2[] = {6000};
    ArkUI_AttributeItem itemHeight2 = {valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(custom_arkui_node, NODE_HEIGHT, &itemHeight2);

    // 创建 RenderNode 并指定尺寸
    auto custom_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(custom_render_node, 300, 300);
    // 将 RenderNode 挂载到自定义节点
    OH_ArkUI_RenderNodeUtils_AddRenderNode(custom_arkui_node, custom_render_node);

    // 用户数据结构体，保存渲染状态
    struct RenderNodeState {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;
        ArkUI_RenderNodeHandle inner_node;
        int count;
    };
    // 动态分配用户数据对象
    RenderNodeState *state = new RenderNodeState;
    // 创建浮点属性用于宽度
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    state->width = widthProperty;
    // 创建浮点属性用于高度
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    state->height = heightProperty;
    // 创建向量属性
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    state->v2 = vectorP;
    // 创建颜色属性
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    state->color = colorP;
    // 保存 RenderNode 句柄
    state->inner_node = custom_render_node;
    // 初始化点击计数
    state->count = 0;


    // 创建上下文回调用于动画更新
    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = state;
    update->callback = [](void *state) {
        // 动画每帧回调中更新属性值
        RenderNodeState *updateState = (RenderNodeState *)state;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(updateState->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(updateState->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(updateState->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(updateState->color, 0xFF0011FF);
    };

    // 获取动画模块接口
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    // 创建动画完成回调
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *state) {
        // 动画结束打印日志
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    // 创建动画配置对象
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);

    // 配置动画期望帧率区间
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    // 启动动画并绑定回调
    animateApi->animateTo(context, option, update, completeCallback);
    // 注册点击事件
    nodeAPI->registerNodeEvent(custom_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, state);
    // 注册事件派发接收器
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            // 取出用户数据，准备处理点击逻辑
            RenderNodeState *state = (RenderNodeState *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = state->inner_node;
            int count = state->count;
            count++;
            state->count = count;
            if (count % 2 == 0) {
                // 偶数次点击清空子节点
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                // 奇数次点击重新创建内容节点
                auto do_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();

                // 将 modifier 绑定到 RenderNode
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_render_node, modifier);
                auto widthP = state->width;
                auto heightP = state->height;
                auto v2p = state->v2;
                auto colorP = state->color;

                // 附加之前创建的属性句柄
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty(modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty(modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property(modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty(modifier, colorP);

                // 将新节点添加到渲染树
                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_render_node, 300, 300);

                // 设置绘制回调执行实际绘制逻辑
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(
                    modifier, state, [](ArkUI_DrawContext *context, void *state) {
                        RenderNodeState *data = (RenderNodeState *)state;
                        // 准备读取属性值
                        float width = 0;
                        float height = 0;
                        uint32_t color = 0;
                        ArkUI_FloatPropertyHandle w = data->width;
                        ArkUI_FloatPropertyHandle h = data->height;

                        // 读取宽高与颜色属性
                        OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                        OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                        ArkUI_ColorPropertyHandle cp = data->color;
                        OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                        // 获取 Canvas 对象
                        auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                        OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);

                        // 创建起点和终点用于渐变
                        OH_Drawing_Point *startPt = OH_Drawing_PointCreate(20, 20);
                        OH_Drawing_Point *endPt = OH_Drawing_PointCreate(900, 900);
                        // 定义渐变颜色与位置
                        uint32_t colors[] = {0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF};
                        float pos[] = {0.0f, 0.5f, 1.0f};
                        // 创建线性渐变着色器
                        OH_Drawing_ShaderEffect *colorShaderEffect = OH_Drawing_ShaderEffectCreateLinearGradient(
                            startPt, endPt, colors, pos, 3, OH_Drawing_TileMode::CLAMP);
                        // 创建画笔刷并绑定渐变
                        OH_Drawing_Brush *brush = OH_Drawing_BrushCreate();
                        OH_Drawing_BrushSetShaderEffect(brush, colorShaderEffect);
                        // 将画刷附着到画布
                        OH_Drawing_CanvasAttachBrush(canvas, brush);
                        // 创建矩形对象（示例中未直接使用）
                        OH_Drawing_Rect *rect = OH_Drawing_RectCreate(100, 100, 900, 900);

                        // 创建路径并绘制三角形轮廓
                        auto rect_path = OH_Drawing_PathCreate();
                        OH_Drawing_PathMoveTo(rect_path, 100, 0);
                        OH_Drawing_PathLineTo(rect_path, width / 4, height * 3 / 4);
                        OH_Drawing_PathLineTo(rect_path, width, height);
                        OH_Drawing_PathClose(rect_path);

                        // 计算五角星坐标
                        auto height_ = 1800;
                        auto width_ = 1800;
                        auto len = height_ / 4;
                        auto aX = width_ / 3;
                        auto aY = height_ / 6;
                        auto dX = aX - len * sin(18.0);
                        auto dY = aY + len * cos(18.0);
                        auto cX = aX + len * sin(18.0);
                        auto cY = dY;
                        auto bX = aX + len / 2.0;
                        auto bY = aY + sqrt((cX - dX) * (cX - dX) + (len / 2.0) * (len / 2.0));
                        auto eX = aX - (len / 2.0);
                        auto eY = bY;

                        // 构造五角星路径
                        auto custom_path = OH_Drawing_PathCreate();
                        OH_Drawing_PathMoveTo(custom_path, aX, aY);
                        OH_Drawing_PathLineTo(custom_path, bX, bY);
                        OH_Drawing_PathLineTo(custom_path, cX, cY);
                        OH_Drawing_PathLineTo(custom_path, dX, dY);
                        OH_Drawing_PathLineTo(custom_path, eX, eY);
                        OH_Drawing_PathClose(custom_path);

                        // 创建画笔并设置宽度与渐变
                        auto pen = OH_Drawing_PenCreate();
                        OH_Drawing_PenSetWidth(pen, 100);
                        OH_Drawing_PenSetShaderEffect(pen, colorShaderEffect);
                        // 绘制五角星路径
                        OH_Drawing_CanvasDrawPath(canvas, custom_path);

                        // 释放绘制资源
                        OH_Drawing_CanvasDetachBrush(canvas);
                        OH_Drawing_BrushDestroy(brush);
                        OH_Drawing_RectDestroy(rect);
                        OH_Drawing_ShaderEffectDestroy(colorShaderEffect);
                        OH_Drawing_PointDestroy(startPt);
                        OH_Drawing_PointDestroy(endPt);
                    });
            }
        }
    });
    // 将自定义节点加入根节点
    nodeAPI->addChild(root_arkui_node, custom_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle draw_hello_world_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个 TypographyStyle 创建 Typography 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 120);
                    OH_Drawing_SetTextStyleFontWeight(txtStyle, FONT_WEIGHT_400);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 将之前创建的 TextStyle 加入 handler 中
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // 设置文本内容，并将文本添加到 handler 中
                    const char *text = "Hello World Drawing\n";
                    // 此处可以使用 OH_Drawing_TypographyHandlerAddEncodedText 添加不同编码的文本。
                    // OH_Drawing_TypographyHandlerAddText 只支持UTF-8编码的文本
                    OH_Drawing_TypographyHandlerAddText(handler, text);  
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_multilingual_text_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个 TypographyStyle，创建 TypographyCreate 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置 locale 为中文  
                    OH_Drawing_SetTypographyTextLocale(typoStyle, "zh-Hans");  
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyle, FONT_WEIGHT_400);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 将之前创建的 TextStyle 加入 handler 中
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // 设置文本内容，并将文本添加到 handler 中
                    const char *text = "你好，中文\n";
                    OH_Drawing_TypographyHandlerAddText(handler, text);  
                    
                    // 通过 handler 创建一个 Typography
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_multi_line_text_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 设置排版宽度
                    double layoutWidth = 800;
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 50);
                    OH_Drawing_SetTextStyleFontWeight(txtStyle, FONT_WEIGHT_400);
                    // 当断词策略为WORD_BREAK_TYPE_BREAK_HYPHEN时，需要为段落设置语言偏好，段落会在不同语言偏好下呈现不同的文本断词效果
                    // OH_Drawing_SetTextStyleLocale(txtStyle, "en-gb");
                    
                    // 设置文本内容
                    const char *text =
                        "Nunc quis augue viverra, venenatis arcu eu, gravida odio. Integer posuere nisi quis ex pretium, a dapibus "
                        "nisl gravida. Mauris lacinia accumsan enim, non tempus ligula. Mauris iaculis dui eu nisi tristique, in porta "
                        "urna varius. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Mauris "
                        "congue nibh mi, vel ultrices ex volutpat et. Aliquam consectetur odio in libero tristique, a mattis ex "
                        "mollis. Praesent et nisl iaculis, facilisis metus nec, faucibus lacus. Duis nec dolor at nibh eleifend "
                        "tempus. Nunc et enim interdum, commodo eros ac, pretium sapien. Pellentesque laoreet orci a nunc pharetra "
                        "pharetra.";
                    
                    
                    // 创建一个断词策略为 BREAK_ALL 的 TypographyStyle
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置断词策略为 WORD_BREAK_TYPE_BREAK_ALL
                    OH_Drawing_SetTypographyTextWordBreakType(typoStyle, WORD_BREAK_TYPE_BREAK_ALL);
                    // 设置最大行数为 10，行数大于 10 的部分不显示
                    OH_Drawing_SetTypographyTextMaxLines(typoStyle, 10);
                    
                    // 使用之前创建的 FontCollection 和 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // 将之前创建的 TextStyle 加入 handler
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double positionBreakAll[2] = {0, 0};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, positionBreakAll[0], positionBreakAll[1]);
                    
                    // 创建一个断词策略为 BREAK_WORD 的 TypographyStyle
                    // OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // OH_Drawing_SetTypographyTextWordBreakType(typoStyle, WORD_BREAK_TYPE_BREAK_WORD);
                    // OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // OH_Drawing_TypographyHandlerAddText(handler, text);
                    // OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // double positionBreakWord[2] = {0, 100};
                    // OH_Drawing_TypographyPaint(typography, canvas, positionBreakWord[0], positionBreakWord[1]);
                    
                    // 创建一个断词策略为 BREAK_HYPHEN 的 TypographyStyle
                    // OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // OH_Drawing_SetTypographyTextStyle(typoStyle, txtStyle);
                    // OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_LEFT);
                    // OH_Drawing_SetTypographyTextWordBreakType(typoStyle, WORD_BREAK_TYPE_BREAK_HYPHEN);
                    // OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // OH_Drawing_TypographyHandlerAddText(handler, text);
                    // OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // double positionBreakWord[2] = {0, 100};
                    // OH_Drawing_TypographyPaint(typography, canvas, positionBreakWord[0], positionBreakWord[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_decoration_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个TypographyStyle创建Typography时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置文本内容
                    const char *text = "Hello World Drawing\n";
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyleWithDeco = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyleWithDeco, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleWithDeco, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleWithDeco, FONT_WEIGHT_400);
                    // 设置装饰线为 LINE_THROUGH
                    OH_Drawing_SetTextStyleDecoration(txtStyleWithDeco, TEXT_DECORATION_LINE_THROUGH);
                    // 设置装饰线样式为 WAVY
                    OH_Drawing_SetTextStyleDecorationStyle(txtStyleWithDeco, TEXT_DECORATION_STYLE_WAVY);
                    // 设置装饰线颜色
                    OH_Drawing_SetTextStyleDecorationColor(txtStyleWithDeco, OH_Drawing_ColorSetArgb(0xFF, 0x6F, 0xFF, 0xFF));
                    
                    // 创建一个不带装饰线的 TextStyle 用于对比
                    OH_Drawing_TextStyle *txtStyleNoDeco = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyleNoDeco, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleNoDeco, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleNoDeco, FONT_WEIGHT_400);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 加入带有装饰线的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleWithDeco);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    
                    // 后续加入的不带装饰线的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleNoDeco);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyleWithDeco);
                    OH_Drawing_DestroyTextStyle(txtStyleNoDeco);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_font_feature_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个 TypographyStyle，创建 TypographyCreate 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置文本内容
                    const char *text = "1/2 1/3 1/4\n";
                    
                    // 设置文字颜色、大小、字重，不设置TextStyle无法绘制出文本
                    OH_Drawing_TextStyle *txtStyleWithFeature = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyleWithFeature, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleWithFeature, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleWithFeature, FONT_WEIGHT_900);
                    // 设置启用frac font feature，此功能将斜线分隔的数字替换为普通（对角线）分数。
                    OH_Drawing_TextStyleAddFontFeature(txtStyleWithFeature, "frac", 1);
                    
                    // 创建一个不带字体特征的 TextStyle 用于对比
                    OH_Drawing_TextStyle *txtStyleNoFeature = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重。不设置 TextStyle 无法绘制出文本
                    OH_Drawing_SetTextStyleColor(txtStyleNoFeature, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleNoFeature, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleNoFeature, FONT_WEIGHT_900);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 加入带有字体特征的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleWithFeature);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 弹出之前创建的 TextStyle
                    OH_Drawing_TypographyHandlerPopTextStyle(handler);
                    
                    // 后续加入的不带字体特征的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleNoFeature);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 弹出之前创建的 TextStyle
                    OH_Drawing_TypographyHandlerPopTextStyle(handler);
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyleWithFeature);
                    OH_Drawing_DestroyTextStyle(txtStyleNoFeature);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_variable_font_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个 TypographyStyle 创建 Typography 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置文字内容
                    const char *text = "Hello World Drawing\n";
                    
                    OH_Drawing_TextStyle *txtStyleWithVar = OH_Drawing_CreateTextStyle();
                    // 设置可变字体的字重，在字体文件支持的情况下，还可以设置"slnt", "wdth"
                    OH_Drawing_TextStyleAddFontVariation(txtStyleWithVar, "wght", 800);
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyleWithVar, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleWithVar, 100);
                    // 此处设置字重不生效，将被可变字体的字重覆盖
                    OH_Drawing_SetTextStyleFontWeight(txtStyleWithVar, FONT_WEIGHT_400);
                    
                    // 创建一个不带可变字体的 TextStyle 用于对比
                    OH_Drawing_TextStyle *txtStyleNoVar = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyleNoVar, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleNoVar, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleNoVar, FONT_WEIGHT_400);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 加入带有可变字体的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleWithVar);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 弹出之前创建的 TextStyle
                    OH_Drawing_TypographyHandlerPopTextStyle(handler);
                    
                    // 后续加入的不带可变字体的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleNoVar);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 弹出之前创建的 TextStyle
                    OH_Drawing_TypographyHandlerPopTextStyle(handler);
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyleWithVar);
                    OH_Drawing_DestroyTextStyle(txtStyleNoVar);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_text_shadow_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个 TypographyStyle 创建 Typography 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    // 设置文本内容
                    const char *text = "Hello World Drawing\n";
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyleWithShadow = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyleWithShadow, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleWithShadow, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleWithShadow, FONT_WEIGHT_400);
                    // 设置阴影偏移量
                    OH_Drawing_Point *offset = OH_Drawing_PointCreate(1, 1);
                    OH_Drawing_TextShadow *shadow = OH_Drawing_CreateTextShadow();
                    // 为 TextShadow 设置样式
                    OH_Drawing_SetTextShadow(shadow, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00), offset, 10);
                    // 将 TextShadow 加入 TextStyle
                    OH_Drawing_TextStyleAddShadow(txtStyleWithShadow, shadow);
                    
                    // 创建一个不带阴影的 TextStyle 用于对比
                    OH_Drawing_TextStyle *txtStyleNoShadow = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyleNoShadow, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyleNoShadow, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyleNoShadow, FONT_WEIGHT_400);
                    
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 加入带有阴影的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleWithShadow);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    
                    // 后续加入的不带阴影的文本样式
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyleNoShadow);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyleWithShadow);
                    //OH_Drawing_DestroyPoint(offset);
                    OH_Drawing_DestroyTextShadow(shadow);
                    OH_Drawing_DestroyTextStyle(txtStyleNoShadow);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_placeholder_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    // 创建 FontCollection，FontCollection 用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    
                    // 设置文字颜色、大小、字重，不设置 TextStyle 会使用 TypographyStyle 中的默认 TextStyle
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 100);
                    OH_Drawing_SetTextStyleFontWeight(txtStyle, FONT_WEIGHT_400);
                    
                    // 设置文本内容
                    const char *text = "Hello World Drawing\n";
                    
                    // 创建一个 TypographyStyle 创建 Typography 时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置文本对齐方式为居中
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    
                    // 使用 FontCollection 和 之前创建的 TypographyStyle 创建 TypographyCreate。TypographyCreate 用于创建 Typography
                    OH_Drawing_TypographyCreate *handlerWithPlaceholder = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // 创建一个 placeholder，并且初始化其成员变量
                    OH_Drawing_PlaceholderSpan placeholder;
                    placeholder.width = 200.0;
                    placeholder.height = 200.0;
                    placeholder.alignment = ALIGNMENT_ABOVE_BASELINE; // 基线对齐策略
                    placeholder.baseline = TEXT_BASELINE_ALPHABETIC;                  // 使用的文本基线类型
                    placeholder.baselineOffset = 0.0; // 相比基线的偏移量。只有对齐策略是 OFFSET_AT_BASELINE 时生效
                    
                    // 将 placeholder 放在开头
                    OH_Drawing_TypographyHandlerAddPlaceholder(handlerWithPlaceholder, &placeholder);
                    
                    // 将之前创建的 TextStyle 加入 handler
                    OH_Drawing_TypographyHandlerPushTextStyle(handlerWithPlaceholder, txtStyle);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handlerWithPlaceholder, text);
                    
                    OH_Drawing_Typography *typographyWithPlaceholder = OH_Drawing_CreateTypography(handlerWithPlaceholder);
                    OH_Drawing_TypographyLayout(typographyWithPlaceholder, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double positionBreakAll[2] = {0, 0};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typographyWithPlaceholder, canvas, positionBreakAll[0], positionBreakAll[1]);
                    
                    // 创建 OH_Drawing_TypographyCreate
                    OH_Drawing_TypographyCreate *handlerNoPlaceholder = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // 将之前创建的 TextStyle 加入 handler
                    OH_Drawing_TypographyHandlerPushTextStyle(handlerNoPlaceholder, txtStyle);
                    // 将文本添加到 handler 中
                    OH_Drawing_TypographyHandlerAddText(handlerNoPlaceholder, text);
                    
                    OH_Drawing_Typography *typographyNoPlaceholder = OH_Drawing_CreateTypography(handlerNoPlaceholder);
                    OH_Drawing_TypographyLayout(typographyNoPlaceholder, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double positionBreakWord[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typographyNoPlaceholder, canvas, positionBreakWord[0], positionBreakWord[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handlerWithPlaceholder);
                    OH_Drawing_DestroyTypographyHandler(handlerNoPlaceholder);
                    OH_Drawing_DestroyTypography(typographyWithPlaceholder);
                    OH_Drawing_DestroyTypography(typographyNoPlaceholder);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_auto_spacing_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个TypographyStyle创建Typography时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 设置使能自动间距，默认为false
                    OH_Drawing_SetTypographyTextAutoSpace(typoStyle, true);
                    // 设置文字内容
                    const char *text = "test测试©test©测试。";
                    
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重，不设置TextStyle会使用TypographyStyle中的默认TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 100);
                    
                    // 创建FontCollection，FontCollection用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    // 使用FontCollection和之前创建的TypographyStyle创建TypographyCreate。TypographyCreate用于创建Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    
                    // 将文本样式添加到handler中
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // 将文本添加到handler中
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 创建段落
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    // 设置排版宽度
                    double layoutWidth = 1310;
                    // 将段落按照排版宽度进行排版
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 设置文本在画布上绘制的起始位置
                    double position[2] = {0, 1140};
                    // 将文本绘制到画布上
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_gradient_color_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    // 设置文字大小
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 100);
                    // 创建着色器对象，并设置颜色、变化起始点与结束点
                    OH_Drawing_Point *startPt = OH_Drawing_PointCreate(0, 0);
                    OH_Drawing_Point *endPt = OH_Drawing_PointCreate(900, 900);
                    uint32_t colors[] = {0xFFFFFF00, 0xFFFF0000, 0xFF0000FF};
                    float pos[] = {0.0f, 0.5f, 1.0f};
                    OH_Drawing_ShaderEffect *colorShaderEffect =
                        OH_Drawing_ShaderEffectCreateLinearGradient(startPt, endPt, colors, pos, 3, OH_Drawing_TileMode::CLAMP);
                    // 创建画刷对象,并将着色器添加到画刷
                    OH_Drawing_Brush* brush = OH_Drawing_BrushCreate();
                    OH_Drawing_BrushSetShaderEffect(brush, colorShaderEffect);
                    // 将画刷添加到文本样式中
                    OH_Drawing_SetTextStyleForegroundBrush(txtStyle, brush);
                    // 创建排版对象，并绘制
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    const char *text = "Hello World";
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    OH_Drawing_TypographyLayout(typography, 1000);
                    OH_Drawing_TypographyPaint(typography, canvas, 0, 0);
                    
                    // 释放对象
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_ShaderEffectDestroy(colorShaderEffect);
                    OH_Drawing_BrushDestroy(brush);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_vertical_align_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    // 设置垂直对齐方式
                    OH_Drawing_SetTypographyVerticalAlignment(typoStyle, OH_Drawing_TextVerticalAlignment::TEXT_VERTICAL_ALIGNMENT_CENTER);
                    // 设置文字大小
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 30);
                    // 设置文字颜色
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    // 创建排版对象，并绘制
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    const char *text = "VerticalAlignment-center";
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    OH_Drawing_TypographyLayout(typography, 1000);
                    OH_Drawing_TypographyPaint(typography, canvas, 0, 0);
                    
                    // 释放对象
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_superscript_subscript_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_TextStyle *badgeTxtStyle = OH_Drawing_CreateTextStyle();
                    // 设置文字大小
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 30);
                    OH_Drawing_SetTextStyleFontSize(badgeTxtStyle, 30);
                    // 设置文字颜色
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleColor(badgeTxtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    // 使能文本上标
                    OH_Drawing_SetTextStyleBadgeType(badgeTxtStyle, OH_Drawing_TextBadgeType::TEXT_SUPERSCRIPT);
                    // 创建排版对象，并绘制
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    const char *text = "Mass-energy equivalence: E=mc";
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, badgeTxtStyle);
                    const char *badgeText = "2";
                    OH_Drawing_TypographyHandlerAddText(handler, badgeText);
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    OH_Drawing_TypographyLayout(typography, 1000);
                    OH_Drawing_TypographyPaint(typography, canvas, 0, 0);
                    
                    // 释放对象
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTextStyle(badgeTxtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_high_contrast_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    OH_Drawing_TextStyle *badgeTxtStyle = OH_Drawing_CreateTextStyle();
                    // 设置文字大小
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 30);
                    OH_Drawing_SetTextStyleFontSize(badgeTxtStyle, 30);
                    // 设置文字颜色
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleColor(badgeTxtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    // 使能文本上标
                    OH_Drawing_SetTextStyleBadgeType(badgeTxtStyle, OH_Drawing_TextBadgeType::TEXT_SUPERSCRIPT);
                    // 创建排版对象，并绘制
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    const char *text = "Mass-energy equivalence: E=mc";
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, badgeTxtStyle);
                    const char *badgeText = "2";
                    OH_Drawing_TypographyHandlerAddText(handler, badgeText);
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    OH_Drawing_TypographyLayout(typography, 1000);
                    OH_Drawing_TypographyPaint(typography, canvas, 0, 0);
                    
                    // 释放对象
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTextStyle(badgeTxtStyle);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

ArkUI_NodeHandle set_style_copy_draw_display_demo(ArkUI_NativeNodeAPI_1 * nodeAPI, ArkUI_ContextHandle context) {
    ArkUI_NodeHandle root_arkui_node = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue valueWidth[] = {400};
    ArkUI_AttributeItem itemWidth = { valueWidth, sizeof(valueWidth) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_WIDTH, &itemWidth);
    ArkUI_NumberValue valueHeight[] = {400};
    ArkUI_AttributeItem itemHeight = { valueHeight, sizeof(valueHeight) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(root_arkui_node, NODE_HEIGHT, &itemHeight);
    valueHeight[0].u32 = 0xff00f100;
    nodeAPI->setAttribute(root_arkui_node, NODE_BACKGROUND_COLOR, &itemHeight);

    auto message_arkui_node = nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    ArkUI_NumberValue valueWidth2[] = {400};
    ArkUI_AttributeItem itemWidth2 = { valueWidth2, sizeof(valueWidth2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_WIDTH, &itemWidth2);
    ArkUI_NumberValue valueHeight2[] = {600};
    ArkUI_AttributeItem itemHeight2 = { valueHeight2, sizeof(valueHeight2) / sizeof(ArkUI_NumberValue)};
    nodeAPI->setAttribute(message_arkui_node, NODE_HEIGHT, &itemHeight2);

    auto message_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
    OH_ArkUI_RenderNodeUtils_SetSize(message_render_node, 300, 300);
    OH_ArkUI_RenderNodeUtils_AddRenderNode(message_arkui_node, message_render_node);

    struct UserData {
        ArkUI_FloatPropertyHandle width;
        ArkUI_FloatPropertyHandle height;
        ArkUI_Vector2PropertyHandle v2;
        ArkUI_ColorPropertyHandle color;        
        ArkUI_RenderNodeHandle inner_render_node;
        int count;
    };
    UserData* userData = new UserData;
    auto widthProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(500);
    userData->width = widthProperty;
    auto heightProperty = OH_ArkUI_RenderNodeUtils_CreateFloatProperty(1000);
    userData->height = heightProperty;
    auto vectorP = OH_ArkUI_RenderNodeUtils_CreateVector2Property(1000, 1000);
    userData->v2 = vectorP;
    auto colorP = OH_ArkUI_RenderNodeUtils_CreateColorProperty(0xFFFFFF00);
    userData->color = colorP;
    userData->inner_render_node = message_render_node;
    userData->count = 0;
    

    ArkUI_ContextCallback *update = new ArkUI_ContextCallback;
    update->userData = userData;
    update->callback = [](void *user) {
        UserData *data = (UserData*) user;
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->width, 100);
        OH_ArkUI_RenderNodeUtils_SetFloatPropertyValue(data->height, 100);
        OH_ArkUI_RenderNodeUtils_SetVector2PropertyValue(data->v2, 100, 100);
        OH_ArkUI_RenderNodeUtils_SetColorPropertyValue(data->color, 0xFF0011FF);
    };
    
    ArkUI_NativeAnimateAPI_1 *animateApi = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_ANIMATE, ArkUI_NativeAnimateAPI_1, animateApi);
    ArkUI_AnimateCompleteCallback *completeCallback = new ArkUI_AnimateCompleteCallback;
    completeCallback->type = ARKUI_FINISH_CALLBACK_REMOVED;
    completeCallback->callback = [](void *userData) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "completeCallback->callback");
    };
    
    ArkUI_AnimateOption *option = OH_ArkUI_AnimateOption_Create();
    OH_ArkUI_AnimateOption_SetDuration(option, 2000);
    OH_ArkUI_AnimateOption_SetTempo(option, 1.1);
    OH_ArkUI_AnimateOption_SetCurve(option, ARKUI_CURVE_EASE);
    OH_ArkUI_AnimateOption_SetDelay(option, 20);
    OH_ArkUI_AnimateOption_SetIterations(option, 1);
    OH_ArkUI_AnimateOption_SetPlayMode(option, ARKUI_ANIMATION_PLAY_MODE_REVERSE);
    
    ArkUI_ExpectedFrameRateRange *range = new ArkUI_ExpectedFrameRateRange;
    range->max = 120;
    range->min = 10;
    range->expected = 60;
    OH_ArkUI_AnimateOption_SetExpectedFrameRateRange(option, range);
    animateApi->animateTo(context, option, update, completeCallback);
    nodeAPI->registerNodeEvent(message_arkui_node, ArkUI_NodeEventType::NODE_ON_CLICK, 999, userData);
    nodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        int32_t targetId = OH_ArkUI_NodeEvent_GetTargetId(event);
        if (targetId == 999) {
            UserData *userData = (UserData *)OH_ArkUI_NodeEvent_GetUserData(event);
            ArkUI_RenderNodeHandle inner_render_node = userData->inner_render_node;
            int count = userData-> count;
            count++;
            userData->count = count;
            if (count % 2 == 0) {
                OH_ArkUI_RenderNodeUtils_ClearChildren(inner_render_node);
            } else {
                auto do_draw_render_node = OH_ArkUI_RenderNodeUtils_CreateNode();
                auto modifier = OH_ArkUI_RenderNodeUtils_CreateContentModifier();
                
                OH_ArkUI_RenderNodeUtils_AttachContentModifier(do_draw_render_node, modifier);
                auto widthP = userData->width;
                auto heightP = userData->height;
                auto v2p = userData->v2;
                auto colorP = userData->color;
                
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, widthP);
                OH_ArkUI_RenderNodeUtils_AttachFloatProperty( modifier, heightP);
                OH_ArkUI_RenderNodeUtils_AttachVector2Property( modifier, v2p);
                OH_ArkUI_RenderNodeUtils_AttachColorProperty( modifier, colorP);

                OH_ArkUI_RenderNodeUtils_AddChild(inner_render_node, do_draw_render_node);
                OH_ArkUI_RenderNodeUtils_SetSize(do_draw_render_node, 400, 400);
                
                OH_ArkUI_RenderNodeUtils_SetContentModifierOnDraw(modifier, userData, [](ArkUI_DrawContext *context, void *userData) {
                    UserData *data = (UserData *) userData;
                    float width = 0;
                    float height = 0;
                    uint32_t color = 0;
                    ArkUI_FloatPropertyHandle w = data -> width;
                    ArkUI_FloatPropertyHandle h = data -> height;
                    ArkUI_ColorPropertyHandle cp = data-> color;
                    
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(w, &width);
                    OH_ArkUI_RenderNodeUtils_GetFloatPropertyValue(h, &height);
                    OH_ArkUI_RenderNodeUtils_GetColorPropertyValue(cp, &color);
                    
                    auto *canvas1 = OH_ArkUI_DrawContext_GetCanvas(context);
                    OH_Drawing_Canvas *canvas = reinterpret_cast<OH_Drawing_Canvas *>(canvas1);
                    // 创建一个TypographyStyle，其中创建Typography时需要使用
                    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
                    // 配置段落样式包括：使能自动间距、最大行数、省略号样式、省略号文本、对齐方式
                    // 使能自动间距
                    OH_Drawing_SetTypographyTextAutoSpace(typoStyle, true);
                    // 设置段落最大行数为3行
                    OH_Drawing_SetTypographyTextMaxLines(typoStyle, 3);
                    // 设置省略号模式为尾部省略号
                    OH_Drawing_SetTypographyTextEllipsisModal(typoStyle, ELLIPSIS_MODAL_TAIL);
                    // 设置省略号文本
                    OH_Drawing_SetTypographyTextEllipsis(typoStyle, "...");
                    // 设置对齐方式为居中对齐
                    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
                    
                    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
                    // 设置文字颜色、大小、字重，不设置TextStyle会使用TypographyStyle中的默认TextStyle
                    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
                    OH_Drawing_SetTextStyleFontSize(txtStyle, 100);
                    // 设置文本的装饰线
                    // 添加下划线
                    OH_Drawing_SetTextStyleDecoration(txtStyle, TEXT_DECORATION_UNDERLINE);
                    // 设置装饰线样式为波浪线样式
                    OH_Drawing_SetTextStyleDecorationStyle(txtStyle, ARKUI_TEXT_DECORATION_STYLE_WAVY);
                    // 设置下划线粗细
                    OH_Drawing_SetTextStyleDecorationThicknessScale(txtStyle, 1);
                    // 设置下划线颜色为蓝色
                    OH_Drawing_SetTextStyleDecorationColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0xFF)); 
                    
                    // 设置阴影的颜色、偏移量、模糊半径
                    // 创建阴影对象
                    OH_Drawing_TextShadow *shadow = OH_Drawing_CreateTextShadow();
                    // 设置阴影偏移量
                    OH_Drawing_Point *offset = OH_Drawing_PointCreate(5, 5);
                    // 定义阴影模糊半径
                    double blurRadius = 4;
                    OH_Drawing_SetTextShadow(shadow, OH_Drawing_ColorSetArgb(0xFF, 0xFF, 0x00, 0xFF), offset, blurRadius);
                    
                    // 拷贝阴影对象
                    OH_Drawing_TextShadow *shadowCopy = OH_Drawing_CopyTextShadow(shadow);
                    // 将拷贝出的阴影添加到文本样式中
                    OH_Drawing_TextStyleAddShadow(txtStyle, shadowCopy);
                    
                    // 创建FontCollection，FontCollection用于管理字体匹配逻辑
                    OH_Drawing_FontCollection *fc = OH_Drawing_CreateSharedFontCollection();
                    
                    // 使用FontCollection和之前创建的TypographyStyle创建TypographyCreate。TypographyCreate用于创建Typography
                    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
                    // 将段落一文本样式添加到handler中
                    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
                    // 将段落一文本添加到handler中
                    const char *text = "The text style, paragraph style, and text shadow of the copied text will be exactly the same as those of the original text.";
                    OH_Drawing_TypographyHandlerAddText(handler, text);
                    // 创建段落一，并将段落一按照排版宽度进行排版
                    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
                    double layoutWidth = 1200;
                    OH_Drawing_TypographyLayout(typography, layoutWidth);
                    // 将段落一文本绘制到画布上
                    double position[2] = {0, 500.0};
                    OH_Drawing_TypographyPaint(typography, canvas, position[0], position[1]);
                    
                    // 生成第二段文本，其中，文本样式和段落样式均由第一段文本拷贝而来
                    // 复制文本样式
                    OH_Drawing_TextStyle *textStyleCopy = OH_Drawing_CopyTextStyle(txtStyle);
                    // 复制段落样式
                    OH_Drawing_TypographyStyle *typographyStyleCopy = OH_Drawing_CopyTypographyStyle(typoStyle);
                    
                    // 使用复制的样式创建段落二，后续可以观察段落一和段落二是否绘制效果一致
                    OH_Drawing_TypographyCreate *handlerCopy = OH_Drawing_CreateTypographyHandler(typographyStyleCopy, fc);
                    OH_Drawing_TypographyHandlerPushTextStyle(handlerCopy, textStyleCopy);
                    OH_Drawing_TypographyHandlerAddText(handlerCopy, text);
                    OH_Drawing_Typography *typographyCopy = OH_Drawing_CreateTypography(handlerCopy);
                    OH_Drawing_TypographyLayout(typographyCopy, layoutWidth);
                    // 将段落二文本绘制到画布上
                    double positionCopy[2] = {0, 1200.0};
                    OH_Drawing_TypographyPaint(typographyCopy, canvas, positionCopy[0], positionCopy[1]);
                    
                    // 释放内存
                    OH_Drawing_DestroyFontCollection(fc);
                    OH_Drawing_DestroyTypographyStyle(typoStyle);
                    OH_Drawing_DestroyTextStyle(txtStyle);
                    OH_Drawing_DestroyTypographyHandler(handler);
                    OH_Drawing_DestroyTypography(typography);
                    // 拷贝的段落样式也需要释放内存
                    OH_Drawing_DestroyTypographyStyle(typographyStyleCopy);
                    // 拷贝的文本样式也需要释放内存
                    OH_Drawing_DestroyTextStyle(textStyleCopy);
                    OH_Drawing_DestroyTypographyHandler(handlerCopy);
                    OH_Drawing_DestroyTypography(typographyCopy);
                });
            }
        }
    });
    nodeAPI->addChild(root_arkui_node, message_arkui_node);
    return root_arkui_node;
}

// Napi暴露的创建函数
napi_value Manager::CreateNativeNode(napi_env env, napi_callback_info info) {
    // 校验环境参数
    if (env == nullptr || info == nullptr) {
        return nullptr;
    }
    // 准备获取参数
    size_t argCnt = 2;
    napi_value args[2] = {nullptr};
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        return nullptr;
    }

    // 校验参数个数
    if (argCnt != 2) {
        napi_throw_type_error(env, NULL, "wrong number of arguments 1");
        return nullptr;
    }
    napi_valuetype valuetype;
    if (napi_typeof(env, args[0], &valuetype) != napi_ok) {
        napi_throw_type_error(env, NULL, "wrong number of arguments 5");
        return nullptr;
    }

    // 确保第一个参数是字符串（XComponent ID）
    if (valuetype != napi_string) {
        napi_throw_type_error(env, NULL, "wrong number of arguments 2");
        return nullptr;
    }

    // 读取 XComponent ID 字符串
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    constexpr uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    size_t length;
    if (napi_get_value_string_utf8(env, args[0], idStr, idSize, &length)) {
        napi_throw_type_error(env, NULL, "wrong number of arguments 3");
        return nullptr;
    }

    // 获取 Manager 单例
    auto manager = Manager::GetInstance();
    if (manager == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "manager == nullptr");
        napi_throw_type_error(env, NULL, "wrong number of arguments 4");
        return nullptr;
    }

    // 根据 ID 查找已缓存的 XComponent
    OH_NativeXComponent *component = manager->GetNativeXComponent(idStr);
    if (component == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "component == nullptr");
        return nullptr;
    }

    // 获取节点接口指针
    auto *nodeAPI = reinterpret_cast<ArkUI_NativeNodeAPI_1 *>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));

    // 校验接口可用性
    if (nodeAPI == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "nodeAPI == nullptr");
        return nullptr;
    }

    // 构建节点并挂载到 XComponent
    if (nodeAPI->createNode != nullptr && nodeAPI->addChild != nullptr) {
        ArkUI_NodeHandle testNode;
        ArkUI_ContextHandle context = nullptr;
        // 从参数获取上下文对象
        auto code = OH_ArkUI_GetContextFromNapiValue(env, args[1], &context);
        (void)code; // 避免未使用警告
        // 调用 demo 构建函数
        testNode = render_node_add_event_demo(nodeAPI, context);
//        testNode = draw_hello_world_demo(nodeAPI, context);
//        testNode = set_multilingual_text_demo(nodeAPI, context);
//        testNode = set_multi_line_text_demo(nodeAPI, context);
//        testNode = set_decoration_demo(nodeAPI, context);
//        testNode = set_font_feature_demo(nodeAPI, context);
//        testNode = set_variable_font_demo(nodeAPI, context);
//        testNode = set_text_shadow_demo(nodeAPI, context);
//        testNode = set_placeholder_demo(nodeAPI, context);
//        testNode = set_auto_spacing_demo(nodeAPI, context);
//        testNode = set_gradient_color_demo(nodeAPI, context);
//        testNode = set_vertical_align_demo(nodeAPI, context);
//        testNode = set_superscript_subscript_demo(nodeAPI, context);
//        testNode = set_high_contrast_demo(nodeAPI, context);
//        testNode = set_style_copy_draw_display_demo(nodeAPI, context);
        if (testNode == nullptr) {
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "testNode == nullptr");
        }
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "mytag", "std::exception");
        // 将创建好的节点附着到 XComponent
        OH_NativeXComponent_AttachNativeRootNode(component, testNode);
    }
    return nullptr;
}

// 根据 ID 获取缓存的 XComponent 指针
OH_NativeXComponent *Manager::GetNativeXComponent(const std::string &id) { return nativeXComponentMap_[id]; }

// 设置缓存 XComponent 指针
void Manager::SetNativeXComponent(std::string &id, OH_NativeXComponent *nativeXComponent) {
    if (nativeXComponent == nullptr) {
        return;
    }

    // 如果当前 ID 未缓存，则直接插入
    if (nativeXComponentMap_.find(id) == nativeXComponentMap_.end()) {
        nativeXComponentMap_[id] = nativeXComponent;
        return;
    }

    // 若已有不同实例，则释放旧对象再覆盖
    if (nativeXComponentMap_[id] != nativeXComponent) {
        OH_NativeXComponent *tmp = nativeXComponentMap_[id];
        delete tmp;
        tmp = nullptr;
        nativeXComponentMap_[id] = nativeXComponent;
    }
}

// 获取或创建对应 ID 的容器实例
Container *Manager::GetContainer(std::string &id) {
    if (containerMap_.find(id) == containerMap_.end()) {
        Container *instance = Container::GetInstance(id);
        containerMap_[id] = instance;
        return instance;
    }

    return containerMap_[id];
}


// 从导出的 NAPI 对象中解包原生 XComponent，缓存并触发容器注册回调，与 JS 侧完成绑定
void Manager::Export(napi_env env, napi_value exports) {
    if (env == nullptr || exports == nullptr) {
        return;
    }

    // 读取导出对象中的 native XComponent 属性
    napi_value exportInstance = nullptr;
    if ((napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance)) != napi_ok) {
        return;
    }

    // 从 NAPI 对象解包原生指针
    OH_NativeXComponent *nativeXComponent = nullptr;
    if (napi_unwrap(env, exportInstance, reinterpret_cast<void **>(&nativeXComponent)) != napi_ok) {
        return;
    }

    // 读取 XComponent 的 ID
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(nativeXComponent, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }

    // 将 ID 封装为 string，便于存储
    std::string id(idStr);
    auto manager = Manager::GetInstance();
    if ((manager != nullptr && nativeXComponent != nullptr)) {
        // 缓存 XComponent 指针
        manager->SetNativeXComponent(id, nativeXComponent);
        auto container = manager->GetContainer(id);
        if (container != nullptr) {
            // 为容器注册回调，完成后续交互
            container->RegisterCallback(nativeXComponent);
        }
    }
}
}