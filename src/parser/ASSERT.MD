# 格式保证
## Homepage
<a href="http://openjudge.cn/user/1481664/">个人首页</a>

## UserHomepage
<a class="group-name" href="http://openjudge.cn/user/1481664/in/group-93/">
					<img src="http://static.openjudge.cn/images/default_logo_small.png" width="25" height="25" />NOI</a>

## GroupHomepage
<a href="http://noi.openjudge.cn/">前往小组</a>

## GroupPage
<ul class="current-contest label">
				<li class="contest-info">
				<h3><span>»</span><a href="/2026autumn05/">计概C-2026秋季学期-第5次作业</a>(5题)</h3>
				<span class="over-time">结束时间：2026-10-27 13:00:00</span>
			</li>
					<li class="contest-info">
				<h3><span>»</span><a href="/2024spring/">2026秋练习汇总（提交不计入平时分）</a>(42题)</h3>
				<span class="over-time">结束时间：2026-12-31 00:00:00</span>
			</li>
					<li class="practice-info">
				<h3>
					<span>»</span><a href="/practice/">练习</a>(52题)				</h3>
				<span class="recently-update">最新题目</span>

# ContestPage
<tbody>
			<tr>
						<td class="solved"></td>
						<td class="problem-id"><a href="/2026autumn05/1/">1</a></td>
			<td class="title"><a href="/2026autumn05/1/">最后一个单词长度</a></td>
			            						<td class="ratio">100%</td>
			<td class="accepted"><a href="/2026autumn05/1/statistics/">68</a></td>
			<td class="submissions"><a href="/2026autumn05/1/statistics/">68</a></td>				
		</tr>
			<tr class="alt">
						<td class="solved"></td>
						<td class="problem-id"><a href="/2026autumn05/2/">2</a></td>
			<td class="title"><a href="/2026autumn05/2/">密码合规检测</a></td>
			            						<td class="ratio">95%</td>
			<td class="accepted"><a href="/2026autumn05/2/statistics/">63</a></td>
			<td class="submissions"><a href="/2026autumn05/2/statistics/">66</a></td>
		</tr>
			<tr>
						<td class="solved"></td>
						<td class="problem-id"><a href="/2026autumn05/3/">3</a></td>
			<td class="title"><a href="/2026autumn05/3/">电影排序</a></td>
			            						<td class="ratio">96%</td>
			<td class="accepted"><a href="/2026autumn05/3/statistics/">55</a></td>
			<td class="submissions"><a href="/2026autumn05/3/statistics/">57</a></td>	
		</tr>
			<tr class="alt">
						<td class="solved"></td>
						<td class="problem-id"><a href="/2026autumn05/4/">4</a></td>
			<td class="title"><a href="/2026autumn05/4/">单位换算 by dengxifeng</a></td>
			            						<td class="ratio">96%</td>
			<td class="accepted"><a href="/2026autumn05/4/statistics/">53</a></td>
			<td class="submissions"><a href="/2026autumn05/4/statistics/">55</a></td>		
		</tr>
			<tr>
						<td class="solved"></td>
						<td class="problem-id"><a href="/2026autumn05/5/">5</a></td>
			<td class="title"><a href="/2026autumn05/5/">成绩修改</a></td>
			            						<td class="ratio">77%</td>
			<td class="accepted"><a href="/2026autumn05/5/statistics/">33</a></td>
			<td class="submissions"><a href="/2026autumn05/5/statistics/">43</a></td>				
		</tr>
		</tbody>


## ProblemPage
<div id="pageTitle"><h2>1:最后一个单词长度</h2></div>
<div id="topMenu">
	<ul>
		<li class="current-show"><a href="/2026autumn05/1/">查看</a></li>
		<li ><a href="/2026autumn05/1/submit/">提交</a></li>
				<li ><a href="/2026autumn05/1/statistics/">统计</a></li>
			
		<li ><a href="/2026autumn05/clarify/1/">提问</a></li>
			
	</ul>
</div><div class="problem-page col-9">
	<dl class="problem-params">
		<dt>总时间限制: </dt>
		<dd>20000ms</dd>
			<dt>内存限制: </dt>
		<dd>20000kB</dd>
	</dl>
	<dl class="problem-content">
		<dt>描述</dt>
		<dd><p>给你一个字符串 <code>s</code>，由若干单词组成，单词前后用<strong>一些</strong>空格字符隔开。返回字符串中 <strong>最后一个</strong> 单词的长度。</p>
<p><strong>单词</strong> 是指仅由字母组成、不包含任何空格字符的最大子字符串。</p>
<pre><strong>输入：</strong>Hello World
<strong>输出：</strong>5
<strong>解释：</strong>最后一个单词是World，长度为5。</pre>
<p> </p></dd>
				<dt>输入</dt>
		<dd>fly me   to   the moon</dd>
		<dt>输出</dt>
		<dd>4</dd>
		<dt>样例输入</dt>
		<dd><pre>Hello World</pre></dd>
		<dt>样例输出</dt>
		<dd><pre>5</pre></dd>
				<dt>提示</dt>
		<dd>1 <= s.length <= 104<br />字符串仅有英文字母和空格' '组成<br />字符串中至少存在一个单词<br /><br />一些空格：或许不止一个；或许前后都有</dd>
						</dl>

## SubmitPage
<div id="submit" class="col-9">
		<p>Submit Your Solution for Problem 1: 最后一个单词长度</p>
		<form id="solution_submit" action="/api/solution/submitv2/" method="post">
			<input type="hidden" name="contestId" value="26242" />
			<input type="hidden" name="problemNumber" value="1" />
			<dl>
				<dt></dt>
				<dd><input type="hidden" name="sourceEncode" value="base64"/></dd>
				<dt>编程语言:</dt>
				<dd>
									<input type="radio" name="language" value="Python3" checked="checked" />Python3(3.8)<br />
								</dd>
				<dt class="submit-source" style="float:none">源代码:</dt>
				<dd><textarea id="source" name="source"></textarea></dd>
							</dl>
			<button class="submit-button" type="submit" name="submit">提交</button>	
			<button class="submit-reset" type="reset">重置</button>
		</form> 
	</div>

## ResultPage
<div id="pageTitle"><h2>#53589551提交状态</h2></div>
<div id=main" class="submitStatus col-8">
    <p class="compile-status">
    	状态: <a href="http://dxfjgc.openjudge.cn/2026autumn05/solution/53589551/#compile-error" class="result-ce">Compile Error</a>	</p>
		
		<a name="compile-error"></a>
	<h3 class="h3-compile-status">编译错误信息</h3>
	<pre>************* Module Main
/home/runner/temp/53589551.48629/Main.py:1:0: E0602: Undefined variable &#039;a&#039; (undefined-variable)
</pre>