--function oldevent_438()
    instruct_1(1494,0,1);   --  1(1):[WWW]说: 杨兄，长年待在这古墓中，*你受的了吗？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1495,58,0);   --  1(1):[杨过]说: 习惯就好了，龙儿还从小就*住在这呢，这是我们古墓派*的传统．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1496,0,1);   --  1(1):[WWW]说: 兄弟近来如何？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_3(-2,-2,-2,-2,439,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:当前场景事件编号

    if instruct_9(11,0) ==false then    --  9(9):是否要求加入?是则跳转到:Label0
        instruct_1(1502,0,1);   --  1(1):[WWW]说: 一切顺利，你夫妇俩可还好*吧．
        instruct_0();   --  0(0)::空语句(清屏)
        instruct_1(1503,58,0);   --  1(1):[杨过]说: 托你的福，一切还好．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label0

    instruct_1(1497,0,1);   --  1(1):[WWW]说: 近日旅途有些不顺，此次前*来是想请杨兄加入，助我一*臂之力．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1498,58,0);   --  1(1):[杨过]说: 那有什麽问题，别的没有，*就是有”一臂”．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1499,0,1);   --  1(1):[WWW]说: 杨兄说笑了．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1500,58,0);   --  1(1):[杨过]说: 此次重出江湖，正好试试新*练成的”黯然销魂掌”．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(1501,0,1);   --  1(1):[WWW]说: 那就走吧．
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_20(0,6) ==true then    --  20(14):队伍是否满？否则跳转到:Label1
        instruct_1(175,58,0);   --  1(1):[杨过]说: 你的队伍已满，*我无法加入．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label1

    instruct_14();   --  14(E):场景变黑
    instruct_3(-2,-2,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:当前场景事件编号
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_13();   --  13(D):重新显示场景
    instruct_10(58);   --  10(A):加入人物[杨过]
--end

