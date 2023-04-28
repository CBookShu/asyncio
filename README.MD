## 
    从官方的asyncio拉取，修改如下【注：修改会保证基本的跨平台特性，至少会在ubuntu上测试通过】:
    1.  使用wsapoll实现win32平台的selector
    2.  对socket操作和句柄类型进行兼容
    3.  移除fmt,catch2,nanobench在asyncio中的依赖，使得asyncio本身非依赖任何第三方库,连测试用例的依赖也去掉了。
    4.  使用xmake 来管理项目，简化后续编译流程[cmake 我真的是搞不来]
    
    TODO:
    1.  使用iocp在win32下工作
    2.  调整为header only


[office github](https://github.com/netcan/asyncio)

[office readme](README_SOURCE.md)
