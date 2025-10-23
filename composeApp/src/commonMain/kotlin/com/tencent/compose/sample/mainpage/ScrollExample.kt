package com.tencent.compose.sample.mainpage

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import org.jetbrains.compose.ui.tooling.preview.Preview

@Composable
fun RowItem(index: Int) {
    Box(modifier = Modifier.fillMaxWidth().height(100.dp).background(Color.Blue)) {
    }
}

@Preview
@Composable
fun LazyListExample() {
    val lazyColumnState = rememberLazyListState()
        LazyColumn(
            modifier = Modifier.fillMaxSize().background(Color.Red),
            verticalArrangement = Arrangement.spacedBy(8.dp),
            state = lazyColumnState,
        ) {
            items(12) { index ->
                RowItem(index)
            }
        }
}

@Preview
@Composable
fun ColumnExample() {
    Box(Modifier.fillMaxSize().background(Color.Gray), contentAlignment = Alignment.Center) {
        Column(
            modifier = Modifier.fillMaxSize().background(Color.Red).verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            for (i in 0 ..20) {
                RowItem(i)
            }
        }
    }
}

@Preview
@Composable
fun BoxExample() {
    Box(Modifier.fillMaxSize().background(Color.Gray).verticalScroll(rememberScrollState()), contentAlignment = Alignment.Center) {
        Column(
            modifier = Modifier.fillMaxSize().background(Color.Red),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            for (i in 0 ..11) {
                RowItem(i)
            }
        }
    }
}