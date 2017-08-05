$(document).ready(function(){

    var userLocalStorage=window.localStorage;
    var skinImg = userLocalStorage.getItem('skin');
    $("#app").css("backgroundImage",skinImg);
    $(".top-banner").height($(".top-banner").width()*149/1005);
    var app=new Vue({
        el:"#app",
        data:{
            fail:false,
        	success:true,
        	searchList:'',
            showsPart:'',
            tvsPart:'',
            moviesPart:'',
            newsPart:'',
            shows:'',
            tvs:'',
            movies:'',
            news:'',
            isShow:{
                all:true,
                shows:false,
                tvs:false,
                movies:false,
                news:false
            },
            isActive:{
                all:true,
                shows:false,
                tvs:false,
                movies:false,
                news:false
            }
        },
        methods:{
            getByAjax:function(){
                var that=this;
                $.ajax({
                    /*url: 'http://iface.qiyi.com/openapi/realtime/recommend?app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1',*/
		    url: 'api/recommend',
                    type: "GET",
                    dataType: "json",
                    success:function(data){
                        that.success=true;
                    	that.fail=false;
                    	that.searchList=data.data[0].video_list;
                        that.shows=data.data[4].video_list;
                        that.tvs=data.data[2].video_list;
                        that.movies=data.data[3].video_list;
                        that.news=data.data[1].video_list;
                        that.showsPart=data.data[4].video_list.slice(0,7);//排行主页每栏只显示7个排行
                        that.tvsPart=data.data[2].video_list.slice(0,7);
                        that.moviesPart=data.data[3].video_list.slice(0,7);
                        that.newsPart=data.data[1].video_list.slice(0,7);

                        that.searchList.forEach(function(x){
                            x.soHref='so.html?key=' + x.short_title;
                        });
                        that.shows.forEach(function(x){
                            x.play='iqiyi://mobile/player?aid=' + x.a_id + '&tvid=' + x.tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                        });
                        that.tvs.forEach(function(x){
                            x.play='iqiyi://mobile/player?aid=' + x.a_id + '&tvid=' + x.tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                        });
                        that.movies.forEach(function(x){
                            x.play='iqiyi://mobile/player?aid=' + x.a_id + '&tvid=' + x.tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                        });
                        that.news.forEach(function(x){
                            x.play='iqiyi://mobile/player?aid=' + x.a_id + '&tvid=' + x.tv_id + '&ftype=27&to=3&url=' + encodeURIComponent(location.href);
                        });
                    },
                    error: function(XMLHttpRequest, textStatus, errorThrown) {
                        that.success=false;
                    	that.fail=true;
                        console.log(textStatus);
                    }
                });
            },
            display:function(x){
                for(var v in this.isShow){
                    this.isShow[v]=false;
                }
                this.isShow[x]=true;
                for(var v in this.isActive){
                    this.isActive[v]=false;
                }
                this.isActive[x]=true;
            },
        }
    })
    app.getByAjax();

    $('ul.nav > li').click(function (e) {
		e.preventDefault();
		$('ul.nav > li').removeClass('active');
		$(this).addClass('active');
	});
    function add(x){
        $('ul.nav > li').removeClass('active');
    }

	$('#searchinput').focus(function(){
		$('.searchRecommend').css('display','block');
	})
    var timer = null;
    $("#searchinput").blur(function(){
        timer = setTimeout(function(){
                    $('.searchRecommend').css('display','none');
                },500);
    });
    $(".searchRecommend").mouseover(function(){
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

