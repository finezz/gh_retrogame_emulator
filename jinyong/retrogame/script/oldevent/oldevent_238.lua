--function oldevent_238()

    if instruct_4(194,1,0) ==false then    --  4(4):是否使用物品[烧刀子]？是则跳转到:Label0
        do return; end
    end    --:Label0

    instruct_32(194,-1);   --  32(20):物品[烧刀子]+[-1]
    instruct_27(3,5722,5748);   --  27(1B):显示动画
    instruct_3(-2,-2,-2,-2,237,241,-1,5722,5748,5722,-2,-2,-2);   --  3(3):修改事件定义:当前场景:当前场景事件编号
    instruct_3(-2,2,-2,-2,239,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [2]
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(775,35,0);   --  1(1):[令狐冲]说: 这烧刀子真是辛辣有劲，*可惜美味不足．
    instruct_0();   --  0(0)::空语句(清屏)
--end

