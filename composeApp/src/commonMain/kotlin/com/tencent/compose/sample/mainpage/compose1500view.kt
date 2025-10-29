package com.tencent.compose.sample.mainpage.sectionItem

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import org.jetbrains.compose.resources.ExperimentalResourceApi
import org.jetbrains.compose.ui.tooling.preview.Preview
import kotlin.random.Random

val random = Random

@Suppress("UnusedBoxWithConstraintsScope")
@OptIn(ExperimentalResourceApi::class)
@Preview
@Composable
internal fun ComposeView1500Page() {
    // 1. 创建一个布尔状态，用于触发颜色切换
    var animate by remember { mutableStateOf(true) }

//     2. 使用 LaunchedEffect 创建一个无限循环，每2秒钟翻转一次状态
    LaunchedEffect(Unit) {
        while (true) {
            delay(2000) // 等待2秒
            animate = !animate
        }
    }

    val rootColor by animateColorAsState(
        targetValue = if (animate) Color.LightGray else Color(0xFF4A4A4A),
        animationSpec = tween(durationMillis = 2000)
    )

    // 让1500个矩形在一个屏幕内完整显示，相邻颜色不一致
    BoxWithConstraints(
        modifier = Modifier.fillMaxWidth()
            .fillMaxHeight()
            .background(rootColor)
    ) {
        val boxCount = 1500
        val columns = 30
        val rows = (boxCount + columns - 1) / columns
        val boxWidth = maxWidth / columns
        val boxHeight = maxHeight / rows
        // 定义两种不同的颜色
        val colorA = Color(0.2f, 0.6f, 0.8f, alpha = 0.8f)
        val colorB = Color(0.9f, 0.5f, 0.3f, alpha = 0.8f)
        for (i in 0 until boxCount) {
            val row = i / columns
            val col = i % columns
            // 通过行列和的奇偶性来区分颜色，保证相邻不一样
            val color = if ((row + col) % 2 == 0) colorA else colorB
            Box(
                modifier = Modifier
                    .size(boxWidth, boxHeight)
                    .offset(x = boxWidth * col, y = boxHeight * row)
                    .alpha(0.5f)
                    .background(color)
            )
        }
    }
}