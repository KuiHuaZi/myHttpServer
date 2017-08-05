$(document).ready(function(){
    var userLocalStorage=window.localStorage;
    var skinImg = userLocalStorage.getItem('skin');
    $("#app").css("backgroundImage",skinImg);
    //ajax请求频道数据时的加载动画
var opts = {
      lines: 9
    , length: 4
    , width: 4
    , radius: 6
    , scale: 1
    , corners: 1
    , color: '#000'
    , opacity: 0.25
    , rotate: 0
    , direction: 1
    , speed: 1
    , trail: 60
    , fps: 20
    , zIndex: 2e9
    , className: 'spinner'
    , top: 'auto'
    , left: '95%'
    , shadow: false
    , hwaccel: false
    , position: 'absolute'
}
    var spinner = new Spinner(opts);
    var spinnerNav = new Spinner(opts);

    var app=new Vue({
        el:"#app",
        data:{
            fail:false,
        	success:true,
        	searchList:'',
            recommend:'',
            recommendAfterOrder:'',
            keywords:decodeURI(location.search.substr(5)),
            orderFlag:0,
        },
        methods:{
        	 getList:function(){
                var that=this;
                $.ajax({
                    url: 'http://iface.qiyi.com/openapi/realtime/recommend?app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1',
                    type: "GET",
                    dataType: "json",
                    success:function(data){
                        that.success=true;
                    	that.fail=false;
                    	that.searchList=data.data[0].video_list;                     
                        that.searchList.forEach(function(x){
                            x.soHref='so.html?key=' + x.short_title;
                        });         
                    },               
                });
             },   
            getByAjax:function(){
                var that=this;
                $.ajax({
                    url: 'http://iface.qiyi.com/openapi/batch/search?from=mobile_list&page_size=30&version=7.5&app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1&'+window.location.search.substr(1),
                    type: "GET",
                    dataType: "json",
                    beforeSend: function () {
                        $('.recommend-container').addClass('hide');
                        $('.error').addClass('hide');
                        $("#navLoad").removeClass('hide');
                        var target = $("#spinNavDiv").get(0);
                        spinnerNav.spin(target);
                    },
                    success:function(data){
                        $('.recommend-container').removeClass('hide');
                        $('.error').addClass('hide');
                        $("#navLoad").addClass('hide');

                        that.recommend=data.data;
                        that.recommendAfterOrder=that.recommend;
                        that.recommend.forEach(function(x){
                            x.play='iqiyi://mobile/player?aid=' + x.a_id + '&tvid=' + x.tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                        })
                    },
                    error: function(XMLHttpRequest, textStatus, errorThrown) {
                        $("#navLoad").addClass('hide');
                        $('.recommend-container').addClass('hide');
                        $('.error').removeClass('hide');
                        console.log(textStatus);
                    }
                });
            },

            sortByCount:function(){
                function compare(propertyName) {
                    return function(object1, object2) {
                        var value1 = parseInt(object1[propertyName]);
                        var value2 = parseInt(object2[propertyName]);
                        if (value2 < value1) {
                            return -1;
                        } else if (value2 > value1) {
                            return 1;
                        } else {
                            return 0;
                        }
                    }
                }
                this.orderFlag++;
                if(this.orderFlag%2==0){    //不排序
                    this.recommendAfterOrder=this.recommend;
                }
                else{                       ////排序
                    var after=Array.from(this.recommend);   //浅复制
                    this.recommendAfterOrder=after.sort(compare("play_count"));   
                }
            },
        },
        computed:{
            sortByCount:function () {
                function compare(propertyName) {
                    return function(object1, object2) {
                        var value1 = parseInt(object1[propertyName]);
                        var value2 = parseInt(object2[propertyName]);
                        if (value2 < value1) {
                            return -1;
                        } else if (value2 > value1) {
                            return 1;
                        } else {
                            return 0;
                        }
                    }
                }
                if(this.orderFlag%2==0){    //不排序
                    console.log('****'+orderFlag)
                    return this.recommend;
                }
                else{
                    return this.recommend.sort(compare("play_count"));   
                }
                orderFlag++;
            }
        }
    })
    app.getList();
    app.getByAjax();
    $('#searchinput').focus(function(){
		$('.searchRecommend').css('display','block'); 
	})
    var timer = null;
    $("#searchinput").blur(function(){
        timer = setTimeout(function(){
                    $('.searchRecommend').css('display','none'); 
                },500);
    });
    $('.searchRecommend').mouseover(function(){
        clearTimeout(timer);
    });
    $("#mainSearch").click(function(){
       $("#topsearchinputdiv").submit();
    });
    
    $(".backTop").hide();
 	$(function () {
  		$(window).scroll(function(){
  			if ($(window).scrollTop()>$(window).height()*0.2){
    			$(".backTop").fadeIn(500);
   			}else{
    			$(".backTop").fadeOut(500);
   			}
 		})
 	});
    
})

