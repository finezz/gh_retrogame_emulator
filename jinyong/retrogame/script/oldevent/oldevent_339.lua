--function oldevent_339()
    instruct_1(1150,90,0);   --  1(1):[???]说: 来者何人，*可知这里是凌霄城．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1151,0,1);   --  1(1):[WWW]说: 小弟有事想求见贵派掌门．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1152,90,0);   --  1(1):[???]说: 掌门师叔现下不见客．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1153,0,1);   --  1(1):[WWW]说: 在下并无恶意，*烦请这位大哥通报一声．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1154,90,0);   --  1(1):[???]说: 本派今有大事要办，*快快离去，别在这罗嗦．
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_5(1,0) ==false then    --  5(5):是否选择战斗？是则跳转到:Label0
        do return; end
    end    --:Label0

    instruct_1(1155,0,1);   --  1(1):[WWW]说: 实在对不起，*在下一定得见见贵派掌门．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1156,90,0);   --  1(1):[???]说: 好个家伙！想硬闯是不是？
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_6(58,3,0,0) ==false then    --  6(6):战斗[58]是则跳转到:Label1
        instruct_15(83);   --  15(F):战斗失败，死亡
        do return; end
    end    --:Label1

    instruct_3(-2,0,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [0]
    instruct_3(-2,1,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [1]
    instruct_3(-2,2,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [2]
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_13();   --  13(D):重新显示场景
    instruct_56(1);   --  56(38):提高声望值1
--end

