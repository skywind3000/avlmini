# 简介

AVL 树需要被平反昭雪，这个 avl 实现和 linux 的 rbtree 一样高效


## 编译

gcc -O3 -Wall test_avl.c -o test_avl
gcc -O3 -Wall test_map.cpp -o test_map -lstdc++

## 静态内存测评

静态内存性能比较（预先分配节点），avlnode 和  linux 的 rbtree 一样，潜入结构体模式，不需要任何内存分配：

| 节点数量 | 算法 | 搜索 | 插入 | 删除 | 插入旋转 | 删除旋转 | 树高 |
|---------|------|-----|------|------|---------|---------|------|
| 10,000,000 | avlmini | 1234 | 2141 | 515 | 7053316 | 7053496 | 27 |
| 10,000,000 | linux rbtree | 1266 | 2187 | 496 | 5887217 | 6815235 | 33 |
|  1,000,000 | avlmini | 109 | 188 | 48 | 704626 | 704619 | 23 |
|  1,000,000 | linux rbtree | 125 | 187 | 39 | 588194 | 681424 | 27 |

Linux kernel 的 rbtree 应该不会有大的什么性能问题吧？拿它作为一个标杆比对下应该能说明一些问题，为了排除内存分配的干扰，节点全部预分配，使用静态内存对二者进行评测。

可以看出高度优化的 avl 和 rbtree 确实表现差不多，有时候切换任务这个会快点，有时候那个会快一些。

所谓说 avl 树每次重平衡需要回溯回根节点的纯粹胡扯，根据 avl 的性质不难做出两个推论：

插入更新时：如当前节点的高度没有改变，则上面所有父节点的高度和平衡也不会改变。
删除更新时：如当前节点的高度没有改变，且平衡值在 [-1, 1] 区间则所有父节点的高度和平衡都不会改变。

根据这两个特点，AVL可以不需要像教科书那样，每次插入删除都回溯到根节点，基本上往上走几级也就搞定了，这和 rbtree 的搜索范围类似，所以不要拿 rbtree 比没有优化过的 avl 。

所谓说 rbtree 统计性能更好的，说的是旋转次数普遍比 avl树少吧，这是的确，但是 rbtree 调整平衡的手段除了旋转还有着色啊，大量的判断兄弟节点，父节点，祖节点，噼里啪啦换颜色，这些都被吃了？再说 rbtree 的层高确实比 avl 更高，这些因素加在一起，最终两者的结果仍然差不多。

## 动态内存测评

动态内存性能比较，为了和 stl 的 map 比较，avlmini 和 linux rbtree 在插入节点时都进行了内存分配，这样对 std::map 这种需要 overhead 的容器比较起来才比较公平，同时排除字符串影响 key/value 都用 int，这样测试比较纯粹：

| 节点数量 | 算法 | 搜索 | 插入 | 删除 |
|---------|------|------|-----|------|
| 10,000,000 | avlmini | 1266 | 2852 | 547 |
| 10,000,000 | linux rbtree | 1547 | 2745 | 500 |
| 10,000,000 | std::map | 2241 | 3008 | 578 |
| 1,000,000 | avlmini | 109 | 266 | 44 |
| 1,000,000 | linux rbtree | 110 | 234 | 38 |
| 1,000,000 | std::map | 203 | 265 | 47 |

测试编译器：gcc 5.2.0 (mingw) 自带STL (vs 2017 和 gcc 5.4.0 结果类似）。

我们的 avlmini 性能超同样 rbtree 实现的 std::map 不少，可见 avl 被误会很深。


## 结论

AVL 不比 linux rbtree 差，比 std::map 好很多，类似的结论见：

- [Comparison of Binary Search Trees (BSTs)](https://attractivechaos.wordpress.com/2008/10/02/comparison-of-binary-search-trees/)

- [AVL vs STL Map](http://stlavlmap.sourceforge.net/)


# AVL-HASH

树表混合结构的 key/value 容器，使用封闭寻址哈希表+AVL树保存索引，彻底解决哈希冲突，原理及性能见：

https://zhuanlan.zhihu.com/p/31758048

## 标准测试

标准测试主要测试默认 hash 函数，验证键值基本均匀的情况下，不必传统容器慢：

    gcc -O3 -Wall test_map.cpp -o test_map -lstdc++ -lm

下面是三个不同环境/编译器的性能对比：

| 编译器 | 容器 | 搜索 | 插入 | 删除 |
|-------|------|------|-----|------|
| GCC 5.2.0 (mingw) | avl-hash | 194 | 310 | 871 |
| GCC 5.2.0 (mingw) | std::unordered_map | 255 | 713 | 1794 |
| VS2017 (x86) | avl-hash | 185 | 336 | 845 |
| VS2017 (x86) | std::unordered_map | 176 | 849 | 1281 |
| GCC 5.4.0 (linux 64) | avl-hash | 204 | 590 | 1320 |
| GCC 5.4.0 (linux 64) | std::unordered_map | 338 | 747 | 3385 |


## 冲突测试：

当冲突发生时，和标准容器的性能比较，用下面命令编译，VC 请自己修改，主要要定义一个宏：

    gcc -O3 -Wall -DSAME_HASH test_map.cpp -o test_map_collision -lstdc++ -lm

测试结果：

![](https://github.com/skywind3000/avlmini/raw/master/images/avlmap-search.png)

可以看出搜索对比，横轴表示冲突节点数量，纵轴表示测试耗时，可以看出随着碰撞的增加，树表混合结构的查询时间基本只是从 0毫秒增加到了 1-2毫秒，而 unordered_map 的搜索时间却是抛物线上升到1.4秒了。

![](https://github.com/skywind3000/avlmini/raw/master/images/avlmap-insert.png)

可以看出当冲突发生的时候，std::unordered_map 基本就跪了

![](https://github.com/skywind3000/avlmini/raw/master/images/avlmap-delete.png)

通过上面的工作，我们得到了这个最不坏的哈希表，我们用它做一个类似 redis / mq 的服务，存储百万级别的键值不用太过在意数据哈希值分布不均匀所带来的问题了，也不用担心碰撞攻击会让其性能跌落到深渊。

我们没法完全依赖哈希函数，当哈希函数靠不住时，还得靠哈希表本身，这叫打铁还需自身硬嘛，最终测试基本符合我们的初衷和预期。