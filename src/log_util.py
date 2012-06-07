#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012~2013 Deepin, Inc.
#               2012~2013 Long Wei
#
# Author:     Long Wei <yilang2007lw@gmail.com>
# Maintainer: Long Wei <yilang2007lw@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import logging
import string
logger_level=["debug","info","warning","error","critical"]
handler_level=["debug","info","warning","error","critical"]
log_type=["debug","info","warning","error","critical"]
formatter_type=[]
filter_type=[]

class LogUtil():
    '''log util,user interface:
           create_logger(logger_name,logger_level)
           create_logger_handler(logger,handler_type,handler_level,formatter_type,filter_type)
           do_log_msg(logger,log_type,log_msg)
    '''
    def __init__(self):
        self.logger=""
        self.handler=""
        self.formatter=""
        self.filter=""
        self.log_file_name=""

    def create_logger(self,logger_name,logger_level):
        '''return logger according to logger_name'''
        self.logger=logging.getLogger(logger_name)
        self.set_logger_level(self.logger,logger_level)

        return self.logger

    def set_logger_level(self,logger,logger_level):
        '''set the level of the given logger'''
        if logger_level in ["debug","info","warning","error","critical"]:
            logger_level=string.upper(logger_level)
        else:
            logger_level="WARNING"

        if isinstance(logger,logging.Logger):
            logger.setLevel(logger_level)

    def remove_logger_handler(self,logger,handler):
        '''remove_logger_handler'''
        if isinstance(logger,logging.Logger) and isinstance(handler,logging.Handler):
            logger.removeHandler(handler)

    def create_logger_handler(self,logger,handler_type,handler_level,formatter_type,filter_type):
        '''get handler for the logger'''
        if handler_type=="stream":
            self.handler=logging.StreamHandler()
        elif handler_type=="file":
            self.handler=logging.FileHandler(self.get_log_file_name(logger))
        elif handler_type=="rotate":
            self.handler=logging.RotatingFileHandler(self.get_log_file_name(logger))
        elif handler_type=="timerotate":
            self.handler=logging.TimeRotatingFileHandler(self.get_log_file_name(logger))
        elif handler_type=="smtp":
            self.handler=logging.SMTPHandler()
        else:
            print "currently unsupported handler_type"
            self.handler=logging.StreamHandler()

        self.set_handler_level(self.handler,handler_level)
        self.create_handler_formatter(self.handler,formatter_type)
        self.create_handler_filter(self.handler,filter_type)

        self.add_logger_handler(logger,self.handler)

        return self.handler

    def get_log_file_name(self,logger):
        '''generate file name for the FileHandler to logger'''
        logger_id=str(id(logger))
        import time
        current_time=time.ctime()
        suffix=".txt"
        self.log_file_name=current_time+"_"+logger_id+suffix

        return self.log_file_name

    def add_logger_handler(self,logger,handler):
        """add handler to the logger"""
        if isinstance(logger,logging.Logger) and isinstance(handler,logging.Handler):
            logger.addHandler(handler)
    
    def set_handler_level(self,handler,handler_level):
        '''set the level of the given handler'''
        if handler_level in ["noset","debug","info","warning","error","critical"]:
            handler_level=""
        else:
            handler_level=""
            
        if isinstance(handler,logging.Handler):
            handler.setLevel(handler_level)

    def create_handler_formatter(self,handler,formatter_type):
        '''create formatter for the handler'''
        if formatter_type:
            pass
        else:
            pass

        self.formatter=logging.Formatter(formatter_type)
        self.set_handler_formatter(handler,self.formatter)

        return self.formatter

    def set_handler_formatter(self,handler,formatter):
        '''set formatter for the handler'''
        if isinstance(handler,logging.Handler) and isinstance(formatter,logging.Formatter):
            handler.setFormatter(formatter)
    
    def create_handler_filter(self,handler,filter_type):
        """create filter for the handler        """
        if filter_type:
            pass
        else:
            pass
        self.filter=logging.Filter(filter_type)
        self.set_handler_filter(handler,self.filter)

        return self.filter

    def set_handler_filter(self,handler,filter):
        """set filter for the handler        """
        if isinstance(handler,logging.Handler) and isinstance(filter,logging.Filter):
            handler.addFilter(filter)
    

    def do_log_msg(self,logger,log_type,log_msg):
        '''do actual msg log operation'''
        if not isinstance(logger,logging.Logger):
            print "invalid logger"

        if log_type in ["debug","info","warning","error","critical"]:
            logger.log_type(log_msg)
        else:
            print "invalid log_type"

    def test(self):
        logger=self.create_logger("test_logger",None)
        print logger
        # handler=logging.FileHandler(str(id(logger))+".txt")
        # print handler
        # handler=logging.StreamHandler()
        # # print handler
        # print "add handler:"
        # self.add_logger_handler(logger,handler)

        # print self.get_log_file_name(logger)
        # print self.log_file_name
        # print "remove_logger_handler:"
        # self.remove_logger_handler(logger,handler)
        # print handler
        print id(logger)
if __name__=="__main__":
    lu=LogUtil()
    lu.test()
