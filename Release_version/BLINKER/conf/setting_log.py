import logging
from conf.setting_path import log_path

# 配置log对象
logger = logging.getLogger()
fileHandler = logging.FileHandler(log_path, encoding='utf-8')
streamHandler = logging.StreamHandler()    # 创建一个屏幕控制对象
# 写入文件的格式
formatter_fileHandler = logging.Formatter('[%(asctime)s] - %(name)s - %(levelname)s - %(message)s')
# 显示在屏幕上的内容格式
formatter_streamHandler = logging.Formatter('[%(asctime)s] - %(name)s - %(levelname)s [line:%(lineno)d] : %(message)s')
# 文件操作符
fileHandler.setFormatter(formatter_fileHandler)
# 格式关联
streamHandler.setFormatter(formatter_streamHandler)
# logger 对象 和 文件操作符 关联
logger.addHandler(fileHandler)
logger.addHandler(streamHandler)
logger.setLevel(logging.INFO)


# 示例
# logger.debug('debug message')       # 低级别的 # 排错信息
# logger.info('info message')            # 正常信息
# logger.warning('警告错误')      # 警告信息
# logger.error('error message')          # 错误信息
# logger.critical('critical message') # 高级别的 # 严重错误信息

'''
logging.basicConfig()函数中可通过具体参数来更改logging模块默认行为，可用参数有：

filename：用指定的文件名创建FiledHandler，这样日志会被存储在指定的文件中。
filemode：文件打开方式，在指定了filename时使用这个参数，默认值为“a”还可指定为“w”。
format：指定handler使用的日志显示格式。
datefmt：指定日期时间格式。
level：设置rootlogger（后边会讲解具体概念）的日志级别
stream：用指定的stream创建StreamHandler。可以指定输出到sys.stderr,sys.stdout或者文件(f=open(‘test.log’,’w’))，默认为sys.stderr。若同时列出了filename和stream两个参数，则stream参数会被忽略。

format参数中可能用到的格式化串：
%(name)s Logger的名字
%(levelno)s 数字形式的日志级别
%(levelname)s 文本形式的日志级别
%(pathname)s 调用日志输出函数的模块的完整路径名，可能没有
%(filename)s 调用日志输出函数的模块的文件名
%(module)s 调用日志输出函数的模块名
%(funcName)s 调用日志输出函数的函数名
%(lineno)d 调用日志输出函数的语句所在的代码行
%(created)f 当前时间，用UNIX标准的表示时间的浮 点数表示
%(relativeCreated)d 输出日志信息时的，自Logger创建以 来的毫秒数
%(asctime)s 字符串形式的当前时间。默认格式是 “2003-07-08 16:49:45,896”。逗号后面的是毫秒
%(thread)d 线程ID。可能没有
%(threadName)s 线程名。可能没有
%(process)d 进程ID。可能没有
%(message)s用户输出的消息
'''