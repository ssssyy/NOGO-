#include<iostream>
#include<graphics.h>
#include<cmath>
#include<cstring>
#include<algorithm>
#include<mmsystem.h>
#include<vector>
#include<stack>
#include<queue>
#include<fstream>
#pragma comment(lib,"winmm.lib")


using namespace std;
void BGM();
void ShowHelp();
void JudgeRes();
void OnePlay();
void TwoPlay();
void Play();	//电脑决定下棋的函数，即主要算法部分
void Save();
void Load();
void Save2();
void Load2();
inline pair<int, int> Choose_cross(int x, int y);



const int WID = 1000;
const int HEI = 740;
bool mutex = true;			//为0时不可以用，为1时是空闲状态
bool play_music = false;
int is_over = 0;		//0表示未结束，1表示黑胜，-1表示白胜
int player_color = 0;	//玩家的颜色：0表示黑方，1表示白方
int curcolor = 0;		//双人游戏中本轮的颜色
const int cross_posx[9] = { 307,387,467,549,629,710,791,871,951};
const int cross_posy[9] = { 46,127,207,288,368,449,530,609,690 };
int Board[9][9] = { 0 };	//1 black -1 white 0 empty
int actionR[2];				//记录本方上一步棋
int AIactionR[2];			//人机对战中记录AI的上一步棋



const int MCTSn = 64;
const double C = 1.0 / sqrt(2);

int dxdy[4][2] = { {1,0},{0,-1},{-1,0},{0,1} };
int exey[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };


bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };
int Color;

bool inBorder(int x, int y)
{
	return x >= 0 && y >= 0 && x < 9 && y < 9;
}

//true: has air
bool dfs_air(int fx, int fy)
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (Board[dx][dy] == 0)
				flag = true;
			if (Board[dx][dy] == Board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(dx, dy))
					flag = true;
		}
	}
	return flag;
}

//true: available
bool judgeAvailable(int fx, int fy, int col)
{
	Board[fx][fy] = col;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(fx, fy))
	{
		Board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (Board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(dx, dy))
				{
					Board[fx][fy] = 0;
					return false;
				}
		}
	}
	Board[fx][fy] = 0;
	return true;
}


//true: game end
int CountBu1(int col)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (Board[i][j] == 0 && judgeAvailable(i, j, col))
				return false;
	return true;
}

bool ProcStep(int x, int y, int color, bool checkOnly = false)
{
	if (!inBorder(x, y) || Board[x][y])
		return false;
	if (!judgeAvailable(x, y, color))
		return false;
	if (!checkOnly)
		Board[x][y] = color;
	return true;
}

vector<int> best_choice;

class node
{
public:
	int id;
	int score;
	node() {}
	node(int x, int y, int sco)
	{
		id = x * 9 + y;
		score = sco;
	}
	bool operator < (const node& A) const
	{
		return score > A.score;
	}
};

/*
判断是否还有双方可以争夺的点，如果没有就转为随机策略任选一个合法点
*/
bool createmove()
{
	int a1, a2, b1, b2, a3, b3;
	int da1, da2, db1, db2;
	best_choice.clear();
	vector<node> v;
	bool flag = false;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (judgeAvailable(i, j, Color) && judgeAvailable(i, j, -Color))
			{
				flag = true;
				b1 = CountBu1(Color);
				a1 = CountBu1(-Color);
				Board[i][j] = Color;
				b2 = CountBu1(Color);
				a2 = CountBu1(-Color);
				Board[i][j] = -Color;
				b3 = CountBu1(Color);
				a3 = CountBu1(-Color);
				Board[i][j] = 0;
				da1 = a1 - a2, da2 = a1 - a3, db1 = b1 - b2, db2 = b1 - b3;
				int sco = da1 - db1 - da2 + db2;
				v.push_back(node(i, j, sco));
			}
		}
	}
	if (!flag)	//没有双方都可以下的点了，这时候转为随机策略
	{
		return false;
	}
	sort(v.begin(), v.end());
	int maxstep = -200;
	int len = v.size();
	for (int i = 0; i < len; i++)
	{
		if (v[i].score < maxstep) break;
		if (v[i].score > maxstep)
		{
			maxstep = v[i].score;
		}
		best_choice.push_back(v[i].id);
	}
	return true;
}








void Find(int x, int y, int i, int j, int index, int board[9][9], char block[9][9], char block_Qi[81], char pos[81][2], char grid[9][9], char record[9][9])
{
	//(dx,dy)为空，更新气 
    if (board[i][j] == 0)
    {
        switch (block_Qi[index])
        {  
        //原来没有气，则现在有1气 
        case 0:
            pos[index][0] = i;
            pos[index][1] = j;
            block_Qi[index] = 1;
            if (board[x][y] == 1)	//是自己的 
            {
                grid[i][j] |= 2;
            }
            else	//是对方的 
            {
                grid[i][j] |= 1;
                if (record[i][j] == -1 && board[x][y] == -1) 
                {
                    record[i][j] = index;
                }
            }
            break;
        //原来有1气，则现在不止1气 
        case 1:               
			//统计重复了                                                              
            if (pos[index][0] == i && pos[index][1] == j)
            {
                break;
            }
            block_Qi[index] = 2;
            //自己的两个气都不再是唯一的气 
            if (board[x][y] == 1)
            {
                grid[(int)pos[index][0]][(int)pos[index][1]] |= 4;
                grid[i][j] |= 4;
            }
            //不再是对方唯一气，clear 1 
            else if (record[(int)pos[index][0]][(int)pos[index][1]] == index) 
            {
                grid[(int)pos[index][0]][(int)pos[index][1]] = grid[(int)pos[index][0]][(int)pos[index][1]] / 2 * 2;
                record[(int)pos[index][0]][(int)pos[index][1]] = -1;
            }
            break;
        //原来就不止1气，现在也不止 
        case 2:
            if (board[x][y] == 1)
            {
                grid[i][j] |= 4;
            }
            //不必更新对方非唯一情况 
            break;
        }
    }
}

//递归地搜索每个group的气数，并更新每个格子的气的情况和每一个group的气的情况 
void Pre(int x, int y, int index, int board[9][9], char block[9][9], char block_Qi[81], char pos[81][2], char grid[9][9], char record[9][9])
{
	block[x][y] = index;
	int tx,ty;
	for(int i = 0; i < 4; i++)
	{
		tx = x+dxdy[i][0];
		ty = y+dxdy[i][1];
		//更新(x,y)的(tx,ty)情况 
		if(tx>=0&&ty>=0&&tx<=8&&ty<=8)
			Find(x, y, tx, ty, index, board, block, block_Qi, pos, grid, record);
		//同色，继续搜索 
		if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&block[tx][ty]==-1&&board[x][y]==board[tx][ty])
			Pre(tx, ty, index, board, block, block_Qi, pos, grid, record);
	}
}


//计算每个位置有多少步来估值，返回步数，并更新result数组记录可以下的格子情况用于搜索 
int CountBu(int board[9][9], int result[9][9])
{
    int r = 0;
    int groupCount = 0;
    char block[9][9];    //blockIndex 
    char record[9][9]; 	//如果是对方的唯一气情况用它记录以备查询更新 
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            record[i][j] = -1;
            block[i][j] = -1;
        }
    }
    char block_Qi[81] = {0}; 	//block气数   
    char pos[81][2] = {0}; 		//前两个气位置  
    char grid[9][9] = {0};     //最低三位分别表示对方1气、己方1气、己方非1气 
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
        	//空或者已分组，跳过 
            if (block[i][j] != -1 || board[i][j] == 0)
            {
                continue;
            }
            Pre(i, j, groupCount++, board, block, block_Qi, pos, grid, record);
        }
    }
    
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
        	//有棋子 不能下 
            if (board[i][j] != 0)
            {
                result[i][j] = 0;
                continue;
            }
            //对方唯一气 不合法 不能下 
            if (grid[i][j] % 2 == 1) 
            {
                result[i][j] = 0;
                continue;
            }
            //可以下的位置 
            if (grid[i][j] / 4 == 1) 
            {
                result[i][j] = 1;
                r++;
                continue;
            }
            //己方唯一气，要看下了以后是否还是气 
            if (grid[i][j] / 2 % 2 == 1) 
            {
                if ((i != 0 && board[i - 1][j] == 0) || (j != 0 && board[i][j - 1] == 0) || (i != 8 && board[i + 1][j] == 0) || (j != 8 && board[i][j + 1] == 0))
                {
                    result[i][j] = 1;
                    r++;
                }
                //不能下 
                else
                {
                    result[i][j] = 0;
                }
                continue;
            }
            //四面都被围不能下的点，因此单独判断，因为之前统计气时未统计到这个点 
            if ((i == 0 || board[i - 1][j] == -1) && (j == 0 || board[i][j - 1] == -1) && (i == 8 || board[i + 1][j] == -1) && (j == 8 || board[i][j + 1] == -1))
            {
                result[i][j] = 0;
                continue; 
            }
            //不是任何一方的气 可以下 
            result[i][j] = 1;
            r++;
        }
    }
    return r;
}



class MCT
{
public:
    MCT *parent;	//parent node
    MCT *children[MCTSn];	//children node list
    int board[9][9];		//temp board
    int Move[MCTSn][2]; 	//Next move axis
    int child;             	//序号
    int maxn;           	//访问的个数限制
    int value;            	//被访问的得分，UCB算法
    int visn;            	//当前访问计数，用于UCB算法       
    double score;           //当前结点估值得分
    int *times;             //访问的点的总数      
	
	//constructor
    MCT(){}
    MCT(int pre_board[9][9], int nex[2], MCT *p, int *countp):value(0),visn(0),child(0)
    {
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                board[i][j] = -pre_board[i][j]; 
            }
        }
        board[nex[0]][nex[1]] = -1; 
        parent = p;
        times = countp;
        best_child(); 	//新扩展结点就更新沿途结点
    }
    
    //选出最好的一步 
    MCT *Best()
    {
        if (maxn == 0) return this;
        if (maxn > child) 
        {
            MCT *c = new MCT(board, Move[child], this, times);
            children[child++] = c;
            return c;
        }

        int bestChild = 0;
        double maxScore = 0;
        for (int i = 0; i < child; i++)
        {
            if (maxScore < children[i]->score)
            {
                bestChild = i;
                maxScore = children[i]->score;
            }
        }
        return children[bestChild]->Best();
    }

	//模拟下了这一步后的输赢的概率并转换为分数 
    int Expand()
    {
        int neg_board[9][9]; 
        int result[9][9];

        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                neg_board[i][j] = -board[i][j];
            }
        }
        double a = (double)CountBu(board, result);
        double b = (double)CountBu(neg_board, result);
        double rate;
        if (a == b)
        {
            rate = 0.5;
        }
        else
        {
			if(a>b) rate=1.0-0.5/(a-b+1.0);
			else rate=0.5/(b-a+1.0);
        }
        if (rate * (double)RAND_MAX >= (double)rand())
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

	//将叶子结点的score反向传播直到根 
    void backup(int flag)
    {
        MCT *node = this;
        int d = 0;
        while (node != NULL)	//ROOT
        {
            if (flag == d % 2) 
            {
                node->value++;
            }
            node->visn++;
            //UCB估值
            node->score = double(node->value)/double(node->visn)+C*sqrt(2*log(double(*times))/double(node->visn)); 
            node = node->parent;
            d++;
        }
    }

	//基础分+类MinMax算分
    void best_child()
    {
        int result[9][9];
        int resultTemp[9][9];
        int neg_board[9][9]; 
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                neg_board[i][j] = -board[i][j];
            }
        }
        int sump = CountBu(board, result);
        int choice[81];
        int visn = 0;
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (result[i][j])
                {
                    choice[visn] = 9 * i + j;
                    visn++;
                }
            }
        }

        if (sump <= MCTSn)
        {
            maxn = sump;
            for (int i = 0; i < sump; i++)
            {
                Move[i][0] = choice[i]/9;
                Move[i][1] = choice[i]%9;
            }
        }
        else
        {
            maxn = MCTSn;
            int positionMark[81]; 	//score
            for (int i = 0; i < sump; i++)
            {
                int x, y;
                x = choice[i] / 9;
                y = choice[i] % 9;
                int score = 0; 
				int tx,ty,ttx,tty;
				//基础分用围眼的思路来进行计分，围眼加分，延伸扣分
				for(int m = 0; m < 4; m++)
				{
					ttx = x+dxdy[m][0], tty = y+dxdy[m][1];
					tx = x+exey[m][0], ty = y+exey[m][1];
					if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&board[tx][ty]!=0) score++;
					if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&board[tx][ty]==1) score--;
				}

                board[x][y] = 1;
                neg_board[x][y] = -1;
				//类似MinMax思路，用本方能走的步数减去对方能走的步数就是自己的得分 
                score += CountBu(board, resultTemp)*4;
                score -= CountBu(neg_board, resultTemp)*4;

                board[x][y] = 0;
                neg_board[x][y] = 0;

                positionMark[i] = score;
            }
			//冒泡，按照得分大小递减排列每个move 
            for (int i = 0; i < MCTSn; i++)
            {
                int temp = positionMark[i];
                int tempi = i;
                for (int j = i; j < sump; j++)
                {
                    if (temp < positionMark[j])
                    {
                        temp = positionMark[j];
                        tempi = j;
                    }
                }
                swap(positionMark[i], positionMark[tempi]);
                swap(choice[i], choice[tempi]);
            }
            for (int i = 0; i < MCTSn; i++)
            {
                Move[i][0] = choice[i] / 9;
                Move[i][1] = choice[i] % 9;
            }
        }
    }



};



void BGM()
{
	play_music = !play_music;
	if (play_music)
	{
		mciSendString("open ./bgm1.mp3 alias BGM", 0, 0, 0);
		mciSendString("play BGM repeat", 0, 0, 0);
	}
	else
	{
		mciSendString("close BGM", 0, 0, 0);
	}
}

void ShowHelp()
{
	cleardevice();
	IMAGE rule;
	loadimage(&rule, "./rule.jpg", WID, HEI, true);
	putimage(0, 0,&rule);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle(WID-100, HEI-100, WID-20, HEI-20);
	settextcolor(BLACK);
	settextstyle(30, 20, "宋体");
	char ss[] = "返回";
	int wid = 80 / 2 - textwidth(ss) / 2;
	int hei = 80 / 2 - textheight(ss) / 2;
	outtextxy(wid + WID-100, hei + HEI-100, ss);
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > WID-100 && mmsg.x < WID-20 && mmsg.y>HEI-100 && mmsg.y < HEI-20)
			{
				clock_t stclock = clock();
				cout << "帮助：返回主页" << endl;
				setfillcolor(RGB(218, 165, 32));
				setlinecolor(BLACK);
				fillrectangle(WID - 100, HEI - 100, WID - 20, HEI - 20);
				outtextxy(wid + WID - 100, hei + HEI - 100, ss);
				while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
				return;
			}
		}
	}
}

inline void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //x为载入图片的X坐标，y为Y坐标
{

	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}


void JudgeRes()
{
	if (is_over == 0) return;
	HWND hnd = GetHWnd();
	if (is_over == 1)
	{
		int choice = MessageBox(hnd, "黑方胜出", "提示", MB_RETRYCANCEL | MB_ICONWARNING);
		if (choice == IDRETRY)
		{
			cout << "黑方胜出" << endl;
			SetWindowText(hnd, "不围棋");
			int cc = MessageBox(hnd, "是否重新开局？", "提示", MB_YESNO);
			if (cc == IDYES)
			{
				cout << "重新开局" << endl;
				return;
			}
			else
			{
				exit(0);
			}
		}
		exit(0);
	}
	else
	{
		int choice = MessageBox(hnd, "白方胜出", "提示", MB_RETRYCANCEL | MB_ICONWARNING);
		if (choice == IDRETRY)
		{
			cout << "白方胜出" << endl;
			SetWindowText(hnd, "不围棋");
			int cc = MessageBox(hnd, "是否重新开局？", "提示", MB_YESNO);
			if (cc == IDYES)
			{
				cout << "重新开局" << endl;
				return;
			}
			else
			{
				exit(0);
			}
		}
		exit(0);
	}
}

int main()
{
	play_music = false;
	player_color = 0;
	initgraph(WID, HEI,SHOWCONSOLE);
	setbkcolor(WHITE);
	IMAGE startbg,voice,novoice;
	loadimage(&novoice, "./novoice.jpg", 60, 60, true);
	loadimage(&voice, "./voice.jpg", 60, 60, true);
	loadimage(&startbg, "./startbg.jpg",WID,HEI,true);
startloc:
	putimage(0, 0, &startbg);
	if (play_music == true)putimage(780, 640, &voice);
	else putimage(780, 640, &novoice);
	setbkmode(TRANSPARENT);
	settextcolor(BLACK);
	settextstyle(80, 50, "华文彩云");
	char sb[] = "不围棋";
	int wid = WID / 2 - textwidth(sb)/2;
	outtextxy(wid, 100, sb);
	settextstyle(60, 40, "Times New Roman");
	char sb2[] = "NOGO";
	wid = WID / 2 - textwidth(sb2)/2;
	outtextxy(wid, 200, sb2);

	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle(WID/2-100, 300, WID/2+100, 350);	//帮助
	fillrectangle(WID/2-100, 380, WID/2+100, 430);	//人机
	fillrectangle(WID/2-100, 460, WID/2+100, 510);	//双人
	fillrectangle(WID/2-100, 540, WID/2+100, 590);	//退出
	settextcolor(BLACK);
	settextstyle(30, 20, "宋体");
	char s1[] = "游戏规则";
	char s2[] = "人机对战";
	char s3[] = "双人对战";
	char s4[] = "退出游戏";
	int width = 200 / 2 - textwidth(s1) / 2;
	int height = 50 / 2 - textheight(s1) / 2;
	outtextxy(width + WID/2-100, height + 300, s1);
	outtextxy(width + WID/2-100, height + 380, s2);
	outtextxy(width + WID/2-100, height + 460, s3);
	outtextxy(width + WID/2-100, height + 540, s4);
	HWND hnd = GetHWnd();

	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG msg = GetMouseMsg();
			if (msg.uMsg==WM_LBUTTONDOWN)
			{
				if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>300 && msg.y < 350)
				{
					clock_t stclock = clock();
					cout << "帮助选项" << endl;
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 300, WID / 2 + 100, 350);
					outtextxy(width + WID / 2 - 100, height + 300, s1);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					ShowHelp();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>380 && msg.y < 430)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 380, WID / 2 + 100, 430);
					outtextxy(width + WID / 2 - 100, height + 380, s2);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "人机对战模式" << endl;
					memset(Board, 0, sizeof(Board));
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					int co = MessageBox(hnd, "是否选择黑方？", "选择你的颜色", MB_YESNO);
					if (co == IDYES)
					{
						player_color = 0;
						Color = -1;
					}
					else
					{
						player_color = 1;
						Color = 1;
					}
					OnePlay();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>460 && msg.y < 510)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 460, WID / 2 + 100, 510);
					outtextxy(width + WID / 2 - 100, height + 460, s3);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "双人对战模式" << endl;
					memset(Board, 0, sizeof(Board));
					memset(actionR, 0, sizeof(actionR));
					TwoPlay();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>540 && msg.y < 590)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 540, WID / 2 + 100, 590);
					outtextxy(width + WID / 2 - 100, height + 540, s4);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "退出" << endl;
					return 0;
				}
				else if (msg.x > 780 && msg.x < 840 && msg.y>640 && msg.y < 700)
				{
					BGM();
					if (play_music == true)putimage(780, 640, &voice);
					else putimage(780, 640, &novoice);
				}
			}
		}
	}
	closegraph();
}

void OnePlay()
{
	is_over = 0;
	cleardevice();
	IMAGE boardimg,whiteimg,voice,novoice;
	setbkmode(TRANSPARENT);
	loadimage(&novoice, "./novoice.jpg", 40, 40, true);
	loadimage(&whiteimg, "./white.jpg", WID - HEI, HEI, true);
	loadimage(&boardimg, "./board1.jpg", HEI, HEI, true);
	loadimage(&voice, "./voice.jpg", 40, 40, true);
	BeginBatchDraw();
	putimage(WID - HEI, 0, &boardimg);
	putimage(0, 0, &whiteimg);
	if (play_music == true)putimage(20, HEI-80, &voice);
	else putimage(20, HEI-80, &novoice);
	settextstyle(40, 38, "楷体");
	settextcolor(BLACK);
	char ss[] = "工具栏";
	int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
	outtextxy(wid, 40, ss);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
	fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
	fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
	fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
	fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
	fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

	settextstyle(30, 30, "行书");
	char s0[] = "重来";
	char s1[] = "悔棋";
	char s2[] = "存档";
	char s3[] = "读档";
	char s4[] = "认输";
	char s5[] = "退出";
	wid = 120 / 2 - textwidth(s1) / 2;
	int hei = 70 / 2 - textheight(s1) / 2;
	int tp = (WID - HEI) / 2 - 60;
	outtextxy(tp + wid, 210 + hei, s0);
	outtextxy(tp + wid, 300 + hei, s1);
	outtextxy(tp + wid, 390 + hei, s2);
	outtextxy(tp + wid, 480 + hei, s3);
	outtextxy(tp + wid, 570 + hei, s4);
	outtextxy(tp + wid, 660 + hei, s5);
	EndBatchDraw();
	IMAGE bchess, wchess;
	loadimage(&wchess, "./白棋.png", 74, 74, true);
	loadimage(&bchess, "./黑棋.png", 74, 74, true);
firstlast:
	if (player_color == 1)
	{
		Play();
		FlushMouseMsgBuffer();
		mutex = true;
		if (is_over)
		{
			cout << is_over << endl;
			return;
		}
	}
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>tp&&mmsg.x<tp+120)
			{
				if (mmsg.y > 210 && mmsg.y < 280)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225,225,220));
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					outtextxy(tp + wid, 210 + hei, s0);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：重来" << endl;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					EndBatchDraw();
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					memset(Board, 0, sizeof(Board));
					HWND hnd = GetHWnd();
					MessageBox(hnd, "重新开局！", "提示", MB_OK);
					goto firstlast;
				}
				else if (mmsg.y>300 && mmsg.y < 370)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					outtextxy(tp + wid, 300 + hei, s1);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：悔棋" << endl;
					Board[actionR[0]][actionR[1]] = 0;
					Board[AIactionR[0]][AIactionR[1]] = 0;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();

				}
				else if (mmsg.y > 390 && mmsg.y < 460)
				{
					clock_t stclock = clock();
					Save();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：存档" << endl;
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
				}
				else if (mmsg.y > 480 && mmsg.y < 550)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					outtextxy(tp + wid, 480 + hei, s3);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：读档" << endl;
					Load();
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					HWND hnd = GetHWnd();
					if (player_color == 0)
					{
						MessageBox(hnd, "你的颜色为黑色", "读档提示", MB_OK);
					}
					else
					{
						MessageBox(hnd, "你的颜色为白色", "读档提示", MB_OK);
					}
					is_over = 0;
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
				}
				else if (mmsg.y > 570 && mmsg.y < 640)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					outtextxy(tp + wid, 570 + hei, s4);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：认输" << endl;
					HWND hnd = GetHWnd();
					int cho = MessageBox(hnd, "是否认输？","认输", MB_YESNO);
					if (cho == IDYES)
					{
						MessageBox(hnd, "玩家认输，电脑胜出！", "游戏结束", MB_OK);
						mutex = true;
						return;
					}
					else
					{
						setfillcolor(YELLOW);
						setlinecolor(WHITE);
						fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
						outtextxy(tp + wid, 570 + hei, s4);
					}
				}
				else if (mmsg.y > 660 && mmsg.y < 730)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);
					outtextxy(tp + wid, 660 + hei, s5);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：退出" << endl;
					exit(0);
				}
			}
			else if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > 20 && mmsg.x<60 && mmsg.y>HEI - 80 && mmsg.y < HEI - 40)
			{
				BGM();
				if (play_music == true)putimage(20, HEI - 80, &voice);
				else putimage(20, HEI - 80, &novoice);
			}

			//点击棋盘落子
			else if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>WID-HEI)
			{
				if (mutex == false) continue;
				mutex = false;
				cout << "mutex!" << endl;
				int tx, ty;
				pair<int, int> tpa=Choose_cross(mmsg.x,mmsg.y);
				tx = (&tpa)->first, ty = (&tpa)->second;
				cout << tx << " " << ty << endl;
				if (Board[tx][ty] != 0)
				{
					HWND hwd = GetHWnd();
					int chong = MessageBox(hwd, "该位置已有棋子！", "提示", MB_OK);
					mutex = true;
					continue;
				}
				if (player_color == 0) drawAlpha(&bchess,cross_posx[tx] - 37, cross_posy[ty] - 37);
				else drawAlpha(&wchess, cross_posx[tx] - 37, cross_posy[ty] - 37);
				
				if (!ProcStep(tx, ty, -Color))	//不合法
				{
					mutex = true;
					if (player_color)
					{
						cout <<"case 1"<< endl;
						is_over = 1;
						JudgeRes();
						return;
					}
					else
					{
						cout << "case 2" << endl;
						is_over = -1;
						JudgeRes();
						return;
					}
				}
				else
				{
					if (CountBu1(Color))
					{
						if (player_color)
						{
							cout << "case 3" << endl;
							is_over = -1;
							JudgeRes();
							return;
						}
						else
						{
							cout << "case 4" << endl;
							is_over = 1;
							JudgeRes();
							return;
						}
					}
				}
				Board[tx][ty] = (player_color ? -1 : 1);
				actionR[0] =tx, actionR[1]=ty;
				if (player_color == 0)
				{
					Play();
					FlushMouseMsgBuffer();
					mutex = true;
					if (is_over)
					{
						cout << is_over << endl;
						return;
					}
					goto firstlast;
				}
				else goto firstlast;
			}
			
		}
	}
}


inline pair<int, int> Choose_cross(int x, int y)
{
	int dx=1000, dy=1000;
	int xx, yy;
	for (int i = 0; i < 9; i++)
	{
		int tp = abs(x - cross_posx[i]);
		if (tp < dx)
		{
			xx = i;
			dx = tp;
		}
		tp = abs(y - cross_posy[i]);
		if (tp < dy)
		{
			yy = i;
			dy = tp;
		}
	}
	return make_pair(xx, yy);
}


void TwoPlay()
{
	is_over = 0;
	cleardevice();
	IMAGE boardimg, whiteimg, voice, novoice;
	setbkmode(TRANSPARENT);
	loadimage(&novoice, "./novoice.jpg", 40, 40, true);
	loadimage(&whiteimg, "./white.jpg", WID - HEI, HEI, true);
	loadimage(&boardimg, "./board1.jpg", HEI, HEI, true);
	loadimage(&voice, "./voice.jpg", 40, 40, true);
	BeginBatchDraw();
	putimage(WID - HEI, 0, &boardimg);
	putimage(0, 0, &whiteimg);
	if (play_music == true)putimage(20, HEI - 80, &voice);
	else putimage(20, HEI - 80, &novoice);
	settextstyle(40, 38, "楷体");
	settextcolor(BLACK);
	char ss[] = "工具栏";
	int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
	outtextxy(wid, 40, ss);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
	fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
	fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
	fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
	fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
	fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

	settextstyle(30, 30, "行书");
	char s0[] = "重来";
	char s1[] = "悔棋";
	char s2[] = "存档";
	char s3[] = "读档";
	char s4[] = "认输";
	char s5[] = "退出";
	wid = 120 / 2 - textwidth(s1) / 2;
	int hei = 70 / 2 - textheight(s1) / 2;
	int tp = (WID - HEI) / 2 - 60;
	outtextxy(tp + wid, 210 + hei, s0);
	outtextxy(tp + wid, 300 + hei, s1);
	outtextxy(tp + wid, 390 + hei, s2);
	outtextxy(tp + wid, 480 + hei, s3);
	outtextxy(tp + wid, 570 + hei, s4);
	outtextxy(tp + wid, 660 + hei, s5);
	EndBatchDraw();
	IMAGE bchess, wchess;
	loadimage(&wchess, "./白棋.png", 74, 74, true);
	loadimage(&bchess, "./黑棋.png", 74, 74, true);
	curcolor = 1;		//从黑方开始先下
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > tp && mmsg.x < tp + 120)
			{
				if (mmsg.y > 210 && mmsg.y < 280)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					outtextxy(tp + wid, 210 + hei, s0);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "双人：重来" << endl;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					EndBatchDraw();
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					memset(Board, 0, sizeof(Board));
					HWND hnd = GetHWnd();
					MessageBox(hnd, "重新开局！", "提示", MB_OK);
					curcolor = 1;
				}
				else if (mmsg.y > 300 && mmsg.y < 370)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					outtextxy(tp + wid, 300 + hei, s1);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "双人：悔棋" << endl;
					Board[actionR[0]][actionR[1]] = 0;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					curcolor *= -1;

				}
				else if (mmsg.y > 390 && mmsg.y < 460)
				{
					clock_t stclock = clock();
					Save2();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "单人：存档" << endl;
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
				}
				else if (mmsg.y > 480 && mmsg.y < 550)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					outtextxy(tp + wid, 480 + hei, s3);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "双人：读档" << endl;
					Load2();
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "楷体");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "行书");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					HWND hnd = GetHWnd();
					if (curcolor == 1)
					{
						MessageBox(hnd, "轮到黑方下棋", "读档提示", MB_OK);
					}
					else
					{
						MessageBox(hnd, "轮到白方下棋", "读档提示", MB_OK);
					}
					is_over = 0;
					memset(actionR, 0, sizeof(actionR));
				}
				else if (mmsg.y > 570 && mmsg.y < 640)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					outtextxy(tp + wid, 570 + hei, s4);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "双人：认输" << endl;
					HWND hnd = GetHWnd();
					int cho = MessageBox(hnd, "是否认输？", "认输", MB_YESNO);
					if (cho == IDYES)
					{
						if (curcolor == 1)
						{
							MessageBox(hnd, "黑方认输，白方胜利！", "游戏结束", MB_OK);
						}
						else
						{
							MessageBox(hnd, "白方认输，黑方胜利！", "游戏结束", MB_OK);
						}
						return;
					}
					else
					{
						setfillcolor(YELLOW);
						setlinecolor(WHITE);
						fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
						outtextxy(tp + wid, 570 + hei, s4);
					}
				}
				else if (mmsg.y > 660 && mmsg.y < 730)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);
					outtextxy(tp + wid, 660 + hei, s5);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "双：退出" << endl;
					exit(0);
				}
			}
			else if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > 20 && mmsg.x<60 && mmsg.y>HEI - 80 && mmsg.y < HEI - 40)
			{
				BGM();
				if (play_music == true)putimage(20, HEI - 80, &voice);
				else putimage(20, HEI - 80, &novoice);
			}
			//点击棋盘落子
			else if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>WID-HEI)
			{
				int tx, ty;
				pair<int, int> tpa = Choose_cross(mmsg.x, mmsg.y);
				tx = (&tpa)->first, ty = (&tpa)->second;
				cout << tx << " " << ty << endl;
				if (Board[tx][ty] != 0)
				{
					HWND hwd = GetHWnd();
					int chong = MessageBox(hwd, "该位置已有棋子！", "提示", MB_OK);
					continue;
				}
				if (curcolor == 1) drawAlpha(&bchess, cross_posx[tx] - 37, cross_posy[ty] - 37);
				else drawAlpha(&wchess, cross_posx[tx] - 37, cross_posy[ty] - 37);

				if (!ProcStep(tx, ty, curcolor))	//不合法
				{
					if (curcolor==-1)
					{
						cout << "case 1" << endl;
						is_over = 1;
						JudgeRes();
						return;
					}
					else
					{
						cout << "case 2" << endl;
						is_over = -1;
						JudgeRes();
						return;
					}
				}
				else
				{
					if (CountBu1(-curcolor))
					{
						if (curcolor==-1)
						{
							cout << "case 3" << endl;
							is_over = -1;
							JudgeRes();
							return;
						}
						else
						{
							cout << "case 4" << endl;
							is_over = 1;
							JudgeRes();
							return;
						}
					}
				}
				Board[tx][ty] = curcolor;
				actionR[0] = tx, actionR[1] = ty;
				curcolor *= -1;
			}

		}
	}
}



void Play()
{
	int p_count = 0;
	srand((unsigned)time(0));
	int MCTSt = (int)(0.9 * (double)CLOCKS_PER_SEC);
	int board[9][9];
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Color == 1)
			{
				board[i][j] = -Board[i][j];
			}
			else
			{
				board[i][j] = Board[i][j];
			}
		}
	}

	MCT root(board, actionR, NULL, &p_count);

	int start = clock();
	while (clock() - start < MCTSt)
	{
		p_count++;
		MCT* node = root.Best();
		int result = node->Expand();
		node->backup(result == 1 ? 1 : 0); //traceback
	}

	int max = 0;
	int maxI = 0;
	int* bestAction = root.Move[0];
	for (int i = 0; i < root.child; i++)
	{
		if (max < root.children[i]->visn)
		{
			maxI = i;
			max = root.children[i]->visn;
			bestAction = root.Move[i];
		}
	}
	int ax = bestAction[0], ay = bestAction[1];
	IMAGE bchess, wchess;
	loadimage(&wchess, "./白棋.png", 74, 74, true);
	loadimage(&bchess, "./黑棋.png", 74, 74, true);
	cout << "AI" <<" "<<ax<<" "<<ay<<endl;
	if (player_color == 0) drawAlpha(&wchess, cross_posx[ax] - 37, cross_posy[ay] - 37);
	else drawAlpha(&bchess, cross_posx[ax] - 37, cross_posy[ay] - 37);
	if (!ProcStep(ax, ay, Color))	//不合法
	{
		if (player_color)
		{
			is_over = -1;
			JudgeRes();
			return;
		}
		else
		{
			is_over = 1;
			JudgeRes();
			return;
		}
	}
	else
	{
		if (CountBu1(-Color))
		{
			if (player_color)
			{
				is_over = 1;
				JudgeRes();
				return;
			}
			else
			{
				is_over = -1;
				JudgeRes();
				return;
			}
		}
	}
	Board[ax][ay] = (player_color ? 1 : -1);
	AIactionR[0] = ax, AIactionR[1] = ay;
	return;
}


void Save()
{
	ofstream outfile("archiveSingle.out");
	if (outfile.is_open())
	{
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (Board[i][j] == 1)
				{
					outfile << "b";
				}
				else if (Board[i][j] == -1)
				{
					outfile << "w";
				}
				else outfile << "o";
			}
		}
		outfile.close();
	}
	else
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
}

void Save2()
{
	ofstream outfile("archiveDouble.out");
	if (outfile.is_open())
	{
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (Board[i][j] == 1)
				{
					outfile << "b";
				}
				else if (Board[i][j] == -1)
				{
					outfile << "w";
				}
				else outfile << "o";
			}
		}
		outfile.close();
	}
	else
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
}
void Load()
{
	int wcount, bcount;
	wcount = bcount = 0;
	ifstream infile("archiveSingle.out");
	if (!infile.is_open())
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
	char s[100];
	infile.getline(s, 99);
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (s[i * 9 + j] == 'w')
			{
				Board[i][j] = -1;
				wcount++;
			}
			else if (s[i * 9 + j] == 'b')
			{
				Board[i][j] = 1;
				bcount++;
			}
			else Board[i][j] = 0;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << Board[i][j] << " ";
		}
		cout << endl;
	}
	if (wcount == bcount)
	{
		player_color = 0;
		Color = -1;
	}
	else
	{
		player_color = 1;
		Color = 1;
	}
	cout << "上一局玩家颜色为";
	cout << (player_color ? "白色" : "黑色") << endl;
}

void Load2()
{
	int wcount, bcount;
	wcount = bcount = 0;
	ifstream infile("archiveDouble.out");
	if (!infile.is_open())
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
	char s[100];
	infile.getline(s, 99);
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (s[i * 9 + j] == 'w')
			{
				Board[i][j] = -1;
				wcount++;
			}
			else if (s[i * 9 + j] == 'b')
			{
				Board[i][j] = 1;
				bcount++;
			}
			else Board[i][j] = 0;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << Board[i][j] << " ";
		}
		cout << endl;
	}
	if (wcount == bcount)
	{
		curcolor = 1;
	}
	else
	{
		curcolor = -1;
	}
	cout << "轮到";
	cout << (curcolor==-1 ? "白方" : "黑方") << "下棋了"<<endl;
}
