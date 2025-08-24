Router 是FSDRServer需要的主要依赖。

FSDRServer的工作流程是：

（初始化）

1. 启动Router。
2. Router.RenderingData 是一个结构体，用来在FSDRServer和Router之间传递数据。在初始化时，需要Router.RenderingData.Channels = kInputChannels

（每帧）

1. Router.RenderingData.RenderContext = pRenderContext ……
2. Router.WaitAndHandleOneRequest() 这个函数的作用让Router阻塞的等待并处理一个请求。

然后继续下一帧。



class Router

- RenderingData 属性可以从外部设置。RendingData保存如下信息：
  - Scene：`ref<IScene>` 类型。
  - RenderContext：`RenderContext*`类型。
  - Channels：`Falcor::ChannelList`类型。
- 启动时创建StreamServer，但只有在WaitAndHandleOneRequest 中才会阻塞等待
- AddHandler方法添加新的Handler，本质上是执行Handler的routerRegister方法。
- on 方法向eventHandlers列表添加新的处理函数，这个是Handler执行的。
- WaitAndHandleOneRequest 方法，从socket中听一个指令，然后提取json指令的type，调用对应的eventHandler，将整个事件的json传入。如果在听指令时socket断开，



class Handler

- routerRegister 方法传入 \*Router 设置为自己的属性。然后开始调用Router->on注册自己 Router.on("setCameraPosition",  一个handler中定义的处理函数，接受json作为参数。)
- handler中有各种处理函数，这些函数都是接受一个json的、可以正常访问Handler属性的普通成员方法。