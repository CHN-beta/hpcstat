为了识别登陆 jykang@hpc.xmu.edu.cn 时使用的密钥，并分密钥统计使用情况，需要启用 SSH agent forwarding。接下来的内容将带领您在 Windows 系统上配置 SSH agent forwarding。

接下来的内容分为三个部分：
1. 启动 Pageant 并添加密钥。**这一步骤每次登陆前都需要执行。**
2. 配置 PuTTY，使 PuTTY 在每次登陆时不直接使用密钥，而是利用 Pageant 完成认证。这一步骤只需要执行一次。
3. 配置 WinSCP，使 WinSCP 在每次登陆时不直接使用密钥，而是利用 Pageant 完成认证。这一步骤只需要执行一次。

**Pageant**:

分为以下几个步骤：

1. 找到 Pageant 程序。Pageant 会随着 PuTTY 一起安装，一般来说您可以直接在开始菜单中搜索 “pageant” 找到它，也可以在 PuTTY 的安装目录中找到它。
2. 启动 Pageant。启动后可能没有任何反应，也可能有一个黑框闪过，这是正常的。只要右下角的系统托盘中出现了 pageant 的图标就可以了。
   
   ![](pageant1.png)

3. 双击 Pageant 图标，打开 Pageant 窗口。选择 “Add Key”，然后选择您的密钥文件。
   
   ![](pageant2.png)

4. 在使用服务器期间保持 Pageant 打开（可以关闭 Pageant 的窗口，但不要在系统托盘中右键退出）。
5. 使用完毕后，在系统托盘中右键退出 Pageant。

> [!TIP]
> 如果您觉得每次打开 Pageant 都要手动添加密钥很麻烦，并且熟悉 Windows 命令行的使用，
>   可以编写一个批处理文件（将下方代码用记事本保存，然后将扩展名从 `.txt` 改为 `.bat`），每次双击该文件即可启动 Pageant 并自动添加密钥：
> 
> `"C:\ProgramData\chocolatey\bin\PAGEANT.EXE" "Z:\.ssh\id_rsa.ppk"`
> 
> 其中第一个引号内为 Pageant 的路径，第二个引号内为您的密钥文件的路径。
> 
> 也可以将该批处理文件放入开机启动项中，使得 Pageant 在开机时自动启动。
> 
> 因为每个人的密钥文件以及 Pageant 的路径都可能不同，所以这里无法提供通用的批处理文件。

**PuTTY**:

1. 在 Connection -> SSH -> Auth，勾选“Attempt authentication using Pageant”和“Allow agent forwarding”。
   
   ![Alt text](putty1.png)

2. 在 Connection -> SSH -> Auth -> Credentials，清空 “Private key file for authentication”，然后保存。
   
   ![Alt text](putty2.png)

**WinSCP**:

1. 在 SSH -> Authentication，勾选 “使用 Pageant 进行认证”，勾选 “允许代理转发”，清空 “密钥文件”，然后保存。
   
   ![Alt text](winscp1.png)
2. (选做)如果您需要通过 WinSCP 打开 PuTTY 的话，需要在 WinSCP 主界面 -> 工具 -> 选项 -> 集成 -> 应用程序路径中，
    在原来的基础上增加 `-A` 参数。
   
   ![Alt text](winscp2.png)
