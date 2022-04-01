# WriteWord_czx

v_czx 机器人复写汉字

## Notice

1. 输入输出图片文件存在 **"Write/data"** 下

2. [skeleton.cpp](https://git.tsinghua.edu.cn/srt/writeword_czx/-/blob/master/Write/skeleton.cpp#L192)为**main**函数


### **2021/2/28**

##### czx添加项目Write_czx_V1.0源文件（cpp）

+ [VS联合编程halcon库配置参考链接](https://www.cnblogs.com/gongdiwudu/p/14059765.html)

+ [cv库安装及配置参考链接](https://blog.csdn.net/weixin_43947197/article/details/106023462)

+ [potrace库安装及配置参考链接](https://blog.csdn.net/xdg_blog/article/details/85110228)

### **2021/3/1**

##### czx完善项目Write_czx_V1.0 （cpp）
+ 在挣扎着安装学习git之后，终于在23：20成功上传了整体项目！！！

+ ~~（太不容易了，git真不好玩）~~

### **2021/3/5**
##### czx更新项目Write_czx_V1.1 （cpp）
+ 增加了输出为**svg格式**的功能

+ 主要增加了[potrace_bmp_to_svg函数](https://git.tsinghua.edu.cn/srt/writeword_czx/-/blob/master/Write/function.h#L284)，在[main函数](https://git.tsinghua.edu.cn/srt/writeword_czx/-/blob/master/Write/skeleton.cpp#L246)调用了

+ [svg格式组成](https://www.w3school.com.cn/svg/svg_path.asp) 主要是**path**元素

+ 输出为svg格式文件之后即可配合RoboDK程序实现
