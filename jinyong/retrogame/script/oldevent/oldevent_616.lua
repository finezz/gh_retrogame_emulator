--function oldevent_616()
    instruct_1(2294,25,0);   --  1(1):[蓝凤凰]说: 公子急着想见我，不知有何*要事？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2295,0,1);   --  1(1):[WWW]说: 你就是教主？不会吧！*这麽的年轻，莫非你也是用*”欧蕾”？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2296,25,0);   --  1(1):[蓝凤凰]说: 公子在说些什麽，小女子怎*麽都听不懂．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2297,0,1);   --  1(1):[WWW]说: 在下此次前来，是想要跟教*主打听一个人的下落．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2298,25,0);   --  1(1):[蓝凤凰]说: 公子要问的是谁？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2299,0,1);   --  1(1):[WWW]说: 韦小宝．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2300,25,0);   --  1(1):[蓝凤凰]说: 公子听谁说他在这里．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2301,0,1);   --  1(1):[WWW]说: 神龙教洪教主．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2302,25,0);   --  1(1):[蓝凤凰]说: 哼！原来是神龙教的爪牙．*想知道韦公子的下落，先打*倒我再说．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2303,0,1);   --  1(1):[WWW]说: 怎麽翻脸跟翻书一样快．
    instruct_0();   --  0(0)::空语句(清屏)

    if instruct_6(98,3,0,0) ==false then    --  6(6):战斗[98]是则跳转到:Label0
        instruct_15(83);   --  15(F):战斗失败，死亡
        do return; end
    end    --:Label0

    instruct_3(-2,-2,-2,-2,617,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:当前场景事件编号
    instruct_3(-2,0,-2,-2,619,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [0]
    instruct_3(-2,1,-2,-2,619,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [1]
    instruct_3(-2,2,-2,-2,619,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [2]
    instruct_3(-2,3,-2,-2,619,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [3]
    instruct_3(-2,4,-2,-2,619,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:当前场景:场景事件编号 [4]
    instruct_3(71,3,-2,-2,611,-1,-1,-2,-2,-2,-2,-2,-2);   --  3(3):修改事件定义:场景[神龙教]:场景事件编号 [3]
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_13();   --  13(D):重新显示场景
    instruct_1(2304,0,1);   --  1(1):[WWW]说: 姑娘为何动不动就出手，若*姑娘不想告知，我也不会为*难的．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2305,25,0);   --  1(1):[蓝凤凰]说: 要杀要剐随便你，别在那假*惺惺了．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2306,0,1);   --  1(1):[WWW]说: 唉！苗族女人性子都这麽冲*吗？我只不过想向韦小宝打*听”鹿鼎记”的下落而已．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2307,25,0);   --  1(1):[蓝凤凰]说: 姓何的贱人呢？*想借神龙教的力量夺回这教*主的位置，怎麽不敢出来．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2308,0,1);   --  1(1):[WWW]说: 你在说些什麽，什麽教主的*位置．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2309,25,0);   --  1(1):[蓝凤凰]说: 难道你不是帮何铁手那个叛*徒来夺我教主之位的．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2310,0,1);   --  1(1):[WWW]说: 不是啊！*我是在找一本叫”鹿鼎记”*的书，洪教主跟我说是被韦*小宝偷去了，而且他就躲在*你这里．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2311,25,0);   --  1(1):[蓝凤凰]说: 韦公子他是去神龙岛盗书没*错，但是”四十二章经”，*而不是你说的”鹿鼎记”．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2312,0,1);   --  1(1):[WWW]说: 真的，那韦小宝他人呢？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2313,25,0);   --  1(1):[蓝凤凰]说: 他啊！他和他那七个美丽的*妻子，一起逍遥去了．*我也不知道他到那去了．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2314,0,1);   --  1(1):[WWW]说: 洪老头为什麽要骗我？
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2315,25,0);   --  1(1):[蓝凤凰]说: 你真笨啊！*被人利用了还不知道．**他是想借你的手来杀了韦公*子，夺回”四十二章经”．**还有就是杀了我好让本教叛*徒何铁手当上教主，并加以*控制．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2316,0,1);   --  1(1):[WWW]说: 可恶，竟敢欺骗我的感情，*非找他算帐不可．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2317,25,0);   --  1(1):[蓝凤凰]说: 小女子刚才不明事理的跟公*子打了一架，真不好意思．**若公子有需要的地方，我蓝*凤凰愿助一臂之力．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2318,0,1);   --  1(1):[WWW]说: 那里那里，我也有错．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2319,25,0);   --  1(1):[蓝凤凰]说: 公子武功，品性，小女子都*很欣赏的紧，真想跟公子一*起闯荡江湖．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_1(2320,0,1);   --  1(1):[WWW]说: ＜苗族女子真大胆，说话真*直接．＞
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_56(1);   --  56(38):提高声望值1
    instruct_8(3);   --  8(8):切换大地图音乐

    if instruct_9(6,0) ==false then    --  9(9):是否要求加入?是则跳转到:Label1
        instruct_1(2322,0,1);   --  1(1):[WWW]说: 姑娘好意心领了，在下一介*莽夫，实不敢耽搁姑娘的青*春．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label1


    if instruct_20(0,6) ==true then    --  20(14):队伍是否满？否则跳转到:Label2
        instruct_1(175,25,0);   --  1(1):[蓝凤凰]说: 你的队伍已满，*我无法加入．
        instruct_0();   --  0(0)::空语句(清屏)
        do return; end
    end    --:Label2

    instruct_1(2321,0,1);   --  1(1):[WWW]说: 能有美人相伴天涯，实乃我*之荣幸．
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_14();   --  14(E):场景变黑
    instruct_3(-2,-2,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2);   --  3(3):修改事件定义:当前场景:当前场景事件编号
    instruct_0();   --  0(0)::空语句(清屏)
    instruct_13();   --  13(D):重新显示场景
    instruct_10(25);   --  10(A):加入人物[蓝凤凰]
--end

