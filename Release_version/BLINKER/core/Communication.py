#!/usr/bin/env python
# -*- coding: utf-8 -*-

from lib.Blinker import Blinker, BlinkerButton, BlinkerText
from lib.Blinker.BlinkerDebug import *
from conf import setting_log as LOG
from conf.setting_path import command_path, data_path
import re

auth = '67e60be6ca56'
# # 获取门当前状态 1为开，0为关


# 获取开门者的身份信息
# [2019-07-03_19:16:39]-[Name of Unlocker]:JieJun_Ren-[Unlocking mode]:face unlock

def getPerson():
    with open(data_path, 'r', encoding='utf-8') as f:
        data = f.readline().strip()
    patt = '.+:(\w+)-.+'
    result = re.match(patt, data)
    if result is not None:
        return result.group(1)
    else:
        return '无'

def getTime():
    with open(data_path, 'r', encoding='utf-8') as f:
        data = f.readline().strip()
    patt = '^\[(.+)\]-.+'
    result = re.match(patt, data)
    if result is not None:
        return result.group(1)
    else:
        return '无'

def getWay():
    with open(data_path, 'r', encoding='utf-8') as f:
        data = f.readline().strip()
    patt = '.+:(.+)$'
    result = re.match(patt, data)
    if result is not None:
        return result.group(1)
    else:
        return '无'



BLINKER_DEBUG.debugAll()

Blinker.mode("BLINKER_WIFI")
Blinker.begin(auth)



# 按键
user = BlinkerText("user")
time = BlinkerText("time")
way = BlinkerText("way")
tex_face = BlinkerText("tex_face")
tex_finger = BlinkerText("tex_finger")
tex_card = BlinkerText("tex_card")
tex_code = BlinkerText("tex_code")

open_door = BlinkerButton("open_door")
email = BlinkerButton("email")

face = BlinkerButton("face")
face_new = BlinkerButton("face_new")
face_delete = BlinkerButton("face_delete")
face_number = BlinkerButton("face_number")

fingerprint = BlinkerButton("finger")
finger_new = BlinkerButton("finger_new")
finger_delete = BlinkerButton("finger_delete")
finger_number = BlinkerButton("finger_number")

card = BlinkerButton("card")
card_new = BlinkerButton("card_new")
card_delete = BlinkerButton("card_delete")
card_number = BlinkerButton("card_number")

password = BlinkerButton("code")
password_change = BlinkerButton("code_change")
password_way = BlinkerButton("code_way")


# BlinkerText
def data_callback(data):
    '''BlinkerText回调函数'''
    LOG.logger.info(getPerson() + '  ' + getTime() + '  ' + getWay())
    user.print("开锁人", getPerson())  # (文本1，文本2)
    time.print("开锁时间", getTime())
    way.print("解锁方式", getWay())


# BlinkerButton
flag = 0
def update_file(ch, file_path=command_path):
    f = open(file_path, 'r', encoding='utf-8')
    global flag
    next(f)
    if int(f.readline().strip()) == flag:
        flag = not flag
    else:
        flag = 0
    f.close()
    f = open(file_path, 'w', encoding='utf-8')
    f.write(ch + '\n' + str(int(flag)))
    f.close()
    LOG.logger.info('command' + ": " + ch)

def open_door_callback(state):
    LOG.logger.info(open_door.name + ": " + state)
    if state == 'tap':
        update_file('O')

def email_callback(state):
    LOG.logger.info(email.name + ": " + state)
    if state == 'tap':
        update_file('e')
#指纹回调函数
def fingerprint_callback(state):
    LOG.logger.info(fingerprint.name + ": " + state)
    if state == 'press' or state == 'off':
        tex_finger.print("关闭")
        update_file('z')
    if state == 'on':
        tex_finger.print("开启")
        update_file('Z')

def finger_new_callback(state):
    LOG.logger.info(finger_new.name + ": " + state)
    if state == 'tap':
        update_file('n')

def finger_delete_callback(state):
    LOG.logger.info(finger_delete.name + ": " + state)
    if state == 'tap':
        update_file('m')

def finger_number_callback(state):
    LOG.logger.info(finger_number.name + ": " + state)
    if state == 'tap':
        update_file('l')
#人脸回调函数
def face_callback(state):
    LOG.logger.info(face.name + ": " + state)
    if state == 'press' or state == 'off':
        tex_face.print("关闭")
        update_file('f')
    if state == 'on':
        tex_face.print("开启")
        update_file('F')

def face_new_callback(state):
    LOG.logger.info(face_new.name + ": " + state)
    if state == 'tap':
        update_file('q')

def face_delete_callback(state):
    LOG.logger.info(face_delete.name + ": " + state)
    if state == 'tap':
        update_file('d')

def face_number_callback(state):
    LOG.logger.info(face_number.name + ": " + state)
    if state == 'tap':
        update_file('t')
#IC卡回调函数
def card_callback(state):
    LOG.logger.info(card.name + ": " + state)
    if state == 'press' or state == 'off':
        tex_card.print("关闭")
        update_file('c')
    if state == 'on':
        tex_card.print("开启")
        update_file('C')

def card_new_callback(state):
    LOG.logger.info(card_new.name + ": " + state)
    if state == 'tap':
        update_file('i')

def card_delete_callback(state):
    LOG.logger.info(card_delete.name + ": " + state)
    if state == 'tap':
        update_file('j')

def card_number_callback(state):
    LOG.logger.info(card_number.name + ": " + state)
    if state == 'tap':
        update_file('k')
#密码回调函数
def password_callback(state):
    LOG.logger.info(password.name + ": " + state)
    if state == 'press' or state == 'off':
        tex_code.print("关闭")
        update_file('p')
    if state == 'on':
        tex_code.print("开启")
        update_file('P')

def password_change_callback(state):
    LOG.logger.info(password_change.name + ": " + state)
    if state == 'tap':
        update_file('b')

def password_way_callback(state):
    LOG.logger.info(password_way.name + ": " + state)
    if state == 'tap':
        update_file('r')


# 心跳包
def heartbeat_callback():
    '''BlinkerText回调函数'''
    LOG.logger.info(getPerson() + '  ' + getTime() + '  ' + getWay())
    user.print("开锁人", getPerson())  # (文本1，文本2)
    time.print("开锁时间", getTime())
    way.print("解锁方式", getWay())

Blinker.attachHeartbeat(heartbeat_callback)
# 注册 BlinkerButton
open_door.attach(open_door_callback)
email.attach(email_callback)

face.attach(face_callback)
face_new.attach(face_new_callback)
face_delete.attach(face_delete_callback)
face_number.attach(face_number_callback)

fingerprint.attach(fingerprint_callback)
finger_new.attach(finger_new_callback)
finger_delete.attach(finger_delete_callback)
finger_number.attach(finger_number_callback)

card.attach(card_callback)
card_new.attach(card_new_callback)
card_delete.attach(card_delete_callback)
card_number.attach(card_number_callback)

password.attach(password_callback)
password_change.attach(password_change_callback)
password_way.attach(password_way_callback)

# 注册 BlinkerText
Blinker.attachData(data_callback)

def main():
    while True:
        Blinker.run()


