## Report(Chinese)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0001](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0001.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0002](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0002.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0003](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0003.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0004](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0004.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0005](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0005.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0006](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0006.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0007](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0007.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0008](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0008.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0009](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0009.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0010](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0010.jpg)

![基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0011](/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/基于强化学习与形状签名的排样学习模型-王子路&羊山_page-0011.jpg)

## Introduction

2D irregular packing problem is a kind of cutting and packing problem. It can be classified as  two-dimensional irregular open dimensional problem according to Wascher's paper.

To obtain a intial result, heuristic criteria is always 

In this respository, we replace 



算法意义：

- [ ] 取代低效的单一标准，避免获得更优初始解
- [ ] 可以应用到实时系统，更优初始解可以降低优化时间
- [ ] 该模型的下降趋势明显，后续可能可以进一步应用到更为复杂的模型



## Architecture of the Network

The architecture of our network mainly consults works by Hu and Bello. 

<img src="/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/image-20200622142925437.png" alt="image-20200622142925437" height="300px" />

Besides, the shape signature  



## Training Procedure







## Results and What we are doing now?

Based on datasets whose polygons have less than 8 edges, we can see the training result will be 



<img src="/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/image-20200622141555895.png" alt="image-20200622141555895" style="zoom:50%;" />

<img src="/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/image-20200622141548533.png" alt="image-20200622141548533" style="zoom:50%;" />



<img src="/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/image-20200622141522842.png" alt="image-20200622141522842" style="zoom: 33%;" />



<img src="/Users/sean/Documents/Projects/My Github/Learn-to-Pack/img/image-20200622141612674.png" alt="image-20200622141612674" style="zoom:50%;" />

As 

