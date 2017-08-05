

 var app=new Vue({
        el:"#app",
        data:{
            message:[
                {"id":"429912600","title":"我的少女时代","short_title":"我的少女时代","img":"http://pic5.qiyipic.com/image/20151218/97/5a/v_109832724_m_601_m1.jpg","sns_score":"7.7","play_count":"152353498","play_count_text":"1.5亿","a_id":"429912600","tv_id":"429912600","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1447862400000","date_format":"2015-11-19","total_num":"1","update_num":"0"},
                {"id":"327450000","title":"朱诺","short_title":"朱诺","img":"http://pic3.qiyipic.com/image/20170627/8b/08/v_108706585_m_601_m4.jpg","sns_score":"8.2","play_count":"1415981","play_count_text":"141.6万","a_id":"327450000","tv_id":"327450000","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1198512000000","date_format":"2007-12-25","total_num":"1","update_num":"0"},
                {"id":"99169900","title":"牛仔裤的夏天","short_title":"牛仔裤的夏天","img":"http://pic6.qiyipic.com/image/20161215/02/04/v_50091699_m_601_m4.jpg","sns_score":"8.3","play_count":"5798070","play_count_text":"579.8万","a_id":"99169900","tv_id":"99169900","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1117555200000","date_format":"2005-06-01","total_num":"1","update_num":"0"},
                {"id":"482003700","title":"谁的青春不迷茫","short_title":"谁的青春不迷茫","img":"http://pic6.qiyipic.com/image/20160604/f1/6c/v_110385641_m_601_m3.jpg","sns_score":"8.2","play_count":"176769941","play_count_text":"1.8亿","a_id":"482003700","tv_id":"482003700","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1461254400000","date_format":"2016-04-22","total_num":"1","update_num":"0"},
                {"id":"112148000","title":"重返十七岁","short_title":"重返十七岁","img":"http://pic5.qiyipic.com/image/20150328/2d/03/d8/v_50221480_m_601_m5.jpg","sns_score":"8.2","play_count":"6314827","play_count_text":"631.5万","a_id":"112148000","tv_id":"112148000","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1239897600000","date_format":"2009-04-17","total_num":"1","update_num":"0"},
                {"id":"246692200","title":"同桌的你","short_title":"同桌的你","img":"http://pic5.qiyipic.com/image/20160627/40/86/v_106541155_m_601_m5.jpg","sns_score":"7.4","play_count":"124682331","play_count_text":"1.2亿","a_id":"246692200","tv_id":"246692200","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1398355200000","date_format":"2014-04-25","total_num":"1","update_num":"0"},
                {"id":"515258100","title":"垫底辣妹","short_title":"垫底辣妹","img":"http://pic2.qiyipic.com/image/20160803/6e/9d/v_110740785_m_601_m1.jpg","sns_score":"8.2","play_count":"45228220","play_count_text":"4522.8万","a_id":"515258100","tv_id":"515258100","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1460563200000","date_format":"2016-04-14","total_num":"1","update_num":"0"},
                {"id":"348318100","title":"匆匆那年","short_title":"匆匆那年","img":"http://pic0.qiyipic.com/image/20150518/91/e3/v_108948270_m_601_m4.jpg","sns_score":"8.1","play_count":"133180517","play_count_text":"1.3亿","a_id":"348318100","tv_id":"348318100","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1417708800000","date_format":"2014-12-05","total_num":"1","update_num":"0"},
                {"id":"99169900","title":"牛仔裤的夏天","short_title":"牛仔裤的夏天","img":"http://pic6.qiyipic.com/image/20161215/02/04/v_50091699_m_601_m4.jpg","sns_score":"8.3","play_count":"5798045","play_count_text":"579.8万","a_id":"99169900","tv_id":"99169900","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1117555200000","date_format":"2005-06-01","total_num":"1","update_num":"0"},
                {"id":"534485300","title":"微微一笑很倾城","short_title":"微微一笑很倾城","img":"http://pic5.qiyipic.com/image/20160914/da/47/v_110943644_m_601_m1.jpg","sns_score":"8.0","play_count":"196761669","play_count_text":"2亿","a_id":"534485300","tv_id":"534485300","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1470931200000","date_format":"2016-08-12","total_num":"1","update_num":"0"},
                {"id":"364322100","title":"何以笙箫默","short_title":"何以笙箫默","img":"http://pic8.qiyipic.com/image/20150529/ec/15/v_109124039_m_601_m8.jpg","sns_score":"7.8","play_count":"166046527","play_count_text":"1.7亿","a_id":"364322100","tv_id":"364322100","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1430323200000","date_format":"2015-04-30","total_num":"1","update_num":"0"},
                {"id":"281844100","title":"录取通知","short_title":"录取通知","img":"http://pic7.qiyipic.com/image/20150811/01/00/v_62818441_m_601_m2.jpg","sns_score":"8.5","play_count":"1454190","play_count_text":"145.4万","a_id":"281844100","tv_id":"281844100","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1155830400000","date_format":"2006-08-18","total_num":"1","update_num":"0"},
                {"id":"144559000","title":"致我们终将逝去的青春","short_title":"致我们终将逝去的青春","img":"http://pic8.qiyipic.com/image/20160519/94/41/v_50545590_m_601_m4.jpg","sns_score":"7.9","play_count":"51830803","play_count_text":"5183.1万","a_id":"144559000","tv_id":"144559000","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1366905600000","date_format":"2013-04-26","total_num":"1","update_num":"0"},
                {"id":"139551900","title":"歌舞青春","short_title":"歌舞青春","img":"http://pic1.qiyipic.com/image/20140908/98/b2/00/v_50495519_m_601_m3.jpg","sns_score":"8.4","play_count":"3292896","play_count_text":"329.3万","a_id":"139551900","tv_id":"139551900","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1137686400000","date_format":"2006-01-20","total_num":"1","update_num":"0"},
                {"id":"576305200","title":"怦然心动","short_title":"怦然心动","img":"http://pic9.qiyipic.com/image/20161220/59/4a/v_111381249_m_601_m1.jpg","sns_score":"8.5","play_count":"7662396","play_count_text":"766.2万","a_id":"576305200","tv_id":"576305200","is_vip":"1","type":"normal","p_type":"1","date_timestamp":"1281024000000","date_format":"2010-08-06","total_num":"1","update_num":"0"},
                {"id":"100506900","title":"初恋这件小事","short_title":"初恋这件小事","img":"http://pic3.qiyipic.com/image/20150202/8c/a1/1d/v_50105069_m_601_m2.jpg","sns_score":"8.9","play_count":"70459089","play_count_text":"7045.9万","a_id":"100506900","tv_id":"100506900","is_vip":"0","type":"normal","p_type":"1","date_timestamp":"1275667200000","date_format":"2010-06-05","total_num":"1","update_num":"0"},
            ]
        },
        methods:{
            getPlayUrl:function(index){
                var url='iqiyi://mobile/player?aid=' + this.message[index].a_id+ '&tvid=' +this.message[index].tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                return url;
            },   
        }
    })
$(document).ready(function(){

    for(var i=0;i<app.message.length;i++){
        var element=$(".content div").eq(i);
        element['0'].firstChild.style.background="url("+app.message[i].img.slice(0,-4)+"_260_360.jpg?sign=iqiyi) no-repeat center";
    }; 
})

