--function oldevent_477()
    instruct_1(1664,53,0);   --  1(1):[段誉]说: 阁下在大理玩的还开心吧？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1665,0,1);   --  1(1):[WWW]说: 大理境内风光明媚，果然是*个好地方．
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_9(6,0) ==false then    --  9(9):是否要求加入?是则跳转到:Label0
        instruct_1(1663,0,1);   --  1(1):[WWW]说: 好了，不打扰兄台了．*他日有缘，再一同游山玩水*吧．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label0

    instruct_1(1660,0,1);   --  1(1):[WWW]说: 不知兄台是否愿与我同行，*前往那无量山一游？
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_28(0,40,100,6,0) ==false then    --  28(1C):判断WWW品德40-100是则跳转到:Label1
        instruct_1(1662,53,0);   --  1(1):[段誉]说: 嗯．．．我还有些事要办，*恐怕无法与阁下同行．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label1


    if instruct_20(0,6) ==true then    --  20(14):队伍是否满？否则跳转到:Label2
        instruct_1(175,53,0);   --  1(1):[段誉]说: 你的队伍已满，*我无法加入．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label2

    instruct_1(1661,53,0);   --  1(1):[段誉]说: 好啊，有个人做伴，路上也*有个照应．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_14();   --  14(E):场景变黑
    instruct_3(-2,0,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [0]
    instruct_3(-2,8,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [8]
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_13();   --  13(D):重新显示场景
    instruct_10(53);   --  10(A):加入人物[段誉]
--end

