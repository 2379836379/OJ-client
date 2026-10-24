========== GroupPage ==========
endtime应匹配<span class="over-time">结束时间：2026-10-27 13:00:00</span>，解析出：2026-10-27 13:00:00，否则为空。
ContestSetInfo去掉extraText这一项。

========== ContestPage ==========
contestPageUrl = 应该是http://dxfjgc.openjudge.cn/2026autumn05/ 而不是 http://dxfjgc.openjudge.cn/，从<a href="/2026autumn05/" class="first">题目</a>解析出相对url后拼接到groupPageUrl。

========== ProblemPage ==========
tried_people = 0，passed_people = 0错误，解析：<dt>尝试人数</dt><dd>68</dd><dt>通过人数</dt><dd>68</dd>，应得到68，68.
