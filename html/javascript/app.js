//回到顶部
$(".backTop").hide();
$(function () {
    $(window).scroll(function(){
        if ($(window).scrollTop()>$(window).height()*0.2){
            $(".backTop").fadeIn(500);
        }else{
            $(".backTop").fadeOut(500);
        }
        if($(document).scrollTop() + $(window).height() == $(document).height()){
            app.loadmore();
        }
    })
});
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
var ref = new Wilddog("https://lmagic.wilddogio.com/iqiyi/");
var app = new Vue({
    el:"#app",
    data:{
        channel:[
            {
                name:"tvs",
                content:'',
                isShow:true,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"films",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"zongyis",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"znimes",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"musics",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"documentary",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"sports",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"entertainments",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"children",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            },
            {
                name:"original",
                content:'',
                isShow:false,
                getflag:false,
                isOrderByScore:false,
                isOrderByPlayCount:false,
                isHasMoreVedio:true
            }
        ],
        channelIndex:0,
        arr:["电视剧","电影","综艺","动漫","音乐","纪录片","体育","娱乐","少儿","原创"],
	    arr1:["tvs","films","shows","cartoons","music","documentary","sports","entertainment","children","creative"],
        arrbackup:["军事","游戏","旅游","时尚","新闻","母婴","健康","直播","商城","文学"],
        message:'',
        topSearchMessage:'奔跑吧兄弟',
        recommend:'',
        titleDatabase:[],
        loadIndex:1,
        isFail:false,
        channelNum:10,
        searchHistory:[],
        sheet:[]
    },
    computed:{
        rowNum:function(){
            var nowShow = this.channel[this.channelIndex].content
            var res = Math.ceil(nowShow.length/6)
            return res>=1? res:1
        },
        pageShowJson:function(){
            if(this.channel[this.channelIndex].getflag == false){
                this.channel[this.channelIndex].getflag = true
                this.getByAjax(this.channelIndex)
            }
            var nowShow = this.channel[this.channelIndex].content
            var scoreorder = this.channel[this.channelIndex].isOrderByScore
            var countorder = this.channel[this.channelIndex].isOrderByPlayCount
            var filterKey = this.message
            var res = nowShow
            if(res.length > 0){
                //搜索框过滤
                if(filterKey){
                    var tmepArr = new Array()
                    for(i=0;i<nowShow.length;i++){
                      if(String(nowShow[i].short_title).indexOf(filterKey) > -1 ){
                        tmepArr.push(nowShow[i])
                      }
                    }
                    res = tmepArr
                }
                //评分排序
                if(scoreorder == true && countorder == false){
                    res.sort(function(a,b){
                        var a = a.sns_score
                        var b = b.sns_score
                        if( a > b ){
                          return -1
                        }else if( a < b ){
                          return 1
                        }else{
                          return 0
                        }
                    })
                }
                //播放次数排序
                if(scoreorder == false && countorder == true){
                    res.sort(function(a,b){
                        var a = parseInt(a.play_count)
                        var b = parseInt(b.play_count)
                        if( a > b ){
                          return -1
                        }else if( a < b ){
                          return 1
                        }else{
                          return 0
                        }
                    })
                }
                return res
            }else{
                return ''
            }
        },
        //顶部搜索框模糊匹配
        titleDatabaseFiltered:function(){
            var res = []
            var alltitle = this.titleDatabase
            var filterKey = this.topSearchMessage
            if(filterKey != ''){
                for(var i=0;i<alltitle.length;i++){
                    if(String(alltitle[i]).indexOf(filterKey) > -1 && res.length <10){
                        res.push(alltitle[i])
                    }
                }
            }
            return res
        }
    },
    methods:{
        setScoreOrder:function(){
            this.channel[this.channelIndex].isOrderByScore = true
            this.channel[this.channelIndex].isOrderByPlayCount = false
        },
        setPlayCountOrder:function(){
            this.channel[this.channelIndex].isOrderByPlayCount = true
            this.channel[this.channelIndex].isOrderByScore = false
        },
        setwhichTypeShow:function(index){
            this.channel[index].isShow = true
            this.channelIndex = index
            for(var i=0;i<this.channel.length;i++){
                if(i != index){
                    this.channel[i].isShow = false
                }
            }
        },
        imgsrc:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                return show[index].img.slice(0,-4)+ "_220_124.jpg?sign=iqiyi"
            }
        },
        showtitle:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                return show[index].short_title
            }
        },
        showReleaseDate:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                return "上映日期："+show[index].date_format
            }
        },
        getshowscore:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                var score = show[index].sns_score
                return score?score:"无"
            }
        },
        getshowplaycount:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                return show[index].play_count_text
            }
        },
        getPlayLink:function(onerow,onecol){
            var show = this.pageShowJson
            var index = (onerow-1)*6 + (onecol-1)
            if(show != '' && index < show.length){
                var aid = show[index].a_id
                var tvid = show[index].tv_id
                return  'iqiyi://mobile/player?aid=' + aid + '&tvid=' + tvid + '&ftype=27&to=3&url=' + encodeURIComponent(window.location.href);
            }
        },
        //根据频道分类获取视频
        getByAjax:function(theIndex){
            var my_type = this.arr1[theIndex]
            var that = this
	        var url='api/index/'+my_type;
            $.ajax({
                /*url: 'http://iface.qiyi.com/openapi/batch/channel?type=detail&mode=11&is_purchase=2&page_size=30&version=7.5&app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1',*/
                type: "GET",
		        url:url,
                /*data:{
                    channel_name:my_type
                },*/
                dataType: "json",
                beforeSend: function () {
                    $("#navLoad").removeClass('hide');
                    var target = $("#spinNavDiv").get(0);
                    spinnerNav.spin(target);
                },
                success:function(data){
                    that.channel[theIndex].content = data.data.video_list
                    for(var i=0;i<data.data.video_list.length;i++){
                        that.titleDatabase.push(data.data.video_list[i].short_title)
                    }
                    that.isFail=false;
                    $("#navLoad").addClass('hide');
                },
                error: function() {
                    that.isFail=true;
                    $("#navLoad").addClass('hide');
                }
            })
        },
        //加载更多视频
        loadmore:function(){
            this.loadIndex = this.loadIndex+1
            var theIndex= this.channelIndex
            var my_type = this.arr[theIndex]
            var that = this
            $.ajax({
                url: 'http://iface.qiyi.com/openapi/batch/channel?type=detail&mode=11&is_purchase=2&page_size=30&version=7.5&app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1',
                type: "GET",
                data:{
                    channel_name:my_type,
                    page_num:that.loadIndex
                },
                dataType: "json",
                beforeSend: function () {
                    //异步请求时spinner出现
                    $("#moreLoad").removeClass('hide');
                    var target = $("#spinDiv").get(0);
                    spinner.spin(target);
                    $("#loadMore").addClass('hide');
                },
                success:function(data){
                    if(data.code == 100000){
                        that.channel[theIndex].content = that.channel[theIndex].content.concat(data.data.video_list)
                        for(var i=0;i<data.data.video_list.length;i++){
                            that.titleDatabase.push(data.data.video_list[i].short_title)
                        }
                    }else if(data.code == 100002){
                        that.channel[theIndex].isHasMoreVedio = false
                    }
                    that.isFail=false;
                    $("#moreLoad").addClass('hide');
                    $("#loadMore").removeClass('hide');
                },
                error: function() {
                    that.isFail=true;
                    $("#moreLoad").addClass('hide');
                    $("#loadMore").removeClass('hide');
                }
            })
        },
        ishasmorevedio:function(){
            return this.channel[this.channelIndex].isHasMoreVedio
        },
        addClassOnDiv:function(event){
            $(event.target).addClass('hide');
            $(event.target).find(".addToSheet").css('display', 'none');
            $(event.target).find(".newSheetDiv").css('display', 'none');
            $(event.target).find(".hasnoSheet").css('display', 'none');
        },
        addClassOnOneFilm:function(event){
            $(event.target).children('.setScorediv').addClass('hide')
        },
        getRecommendByAjax:function(){
            var that = this
            $.ajax({
                url: 'http://iface.qiyi.com/openapi/realtime/recommend?app_k=f0f6c3ee5709615310c0f053dc9c65f2&app_v=8.4&app_t=0&platform_id=12&dev_os=10.3.1&dev_ua=iPhone9,3&dev_hw=%7B%22cpu%22%3A0%2C%22gpu%22%3A%22%22%2C%22mem%22%3A%2250.4MB%22%7D&net_sts=1&scrn_sts=1&psp_uid=%E5%A5%94%E8%B7%91%E5%90%A7&scrn_res=1334*750&scrn_dpi=153600&qyid=87390BD2-DACE-497B-9CD4-2FD14354B2A4&secure_v=1&secure_p=iPhone&core=1&req_sn=1493946331320&req_times=1',
                type: "GET",
                dataType: "json",
                success:function(data){
                    that.recommend = data.data[0].video_list
                }
            })
        },
        setSearchWord:function(text){
            $("#searchinput").val(text)
            $("#searchinput").focus()
            $("#mainSearch").trigger('click')
        },
        userlike:function(event){
            $(event.target).text(parseInt($(event.target).text()) + 1)
        },
        userDontlike:function(event){
            $(event.target).text(parseInt($(event.target).text()) + 1)
        },
        getUserSearchHistory:function(){
            userLocalStorage = window.localStorage
            if(JSON.parse(userLocalStorage.getItem('searchHistory')) == null){
                userLocalStorage.setItem('searchHistory',JSON.stringify([]));
            }
            var searchhistory = JSON.parse(userLocalStorage.getItem('searchHistory'))
            this.searchHistory = searchhistory
        },
        addCollect:function(onerow,onecol,event){
            if(userLocalStorage.getItem('username') != null){
                var userStorageName = userLocalStorage.getItem('username');
                var show = this.pageShowJson;
                var index = (onerow-1)*6 + (onecol-1);
                if(show != '' && index < show.length){
                    var aid = show[index].a_id;
                    var obj={
                        "aid":aid,
                        "tvid":show[index].tv_id,
                        "title":show[index].short_title,
                        "img":show[index].img.slice(0,-4)+ "_220_124.jpg?sign=iqiyi"
                    }
                    ref.child(userStorageName).child("collect").child(aid).set(obj);
                    var x = event.pageX + 10+"px";
                    var y = event.pageY + 10+"px";
                    $("#collectSuccess").css({"display":"inline-block","top":y,"left":x});
                    $("#collectSuccess").fadeOut(2000);
                }
            }else{
                $("#login").trigger('click');
            }
        },
        addMvSheet:function(onerow,onecol,event){
            if(userLocalStorage.getItem('username') != null){
                var userStorageName = userLocalStorage.getItem('username');
                var show = this.pageShowJson;
                var index = (onerow-1)*6 + (onecol-1);
                if(show != '' && index < show.length){
                    var aid = show[index].a_id;
                    var obj={
                        "aid":aid,
                        "tvid":show[index].tv_id,
                        "title":show[index].short_title,
                        "img":show[index].img.slice(0,-4)+ "_220_124.jpg?sign=iqiyi"
                    }
                    var sheetname = $.trim(event.target.innerText);
                    ref.child(userStorageName).child("mvSheet").child(sheetname).child(aid).set(obj);
                    var x = event.pageX + 10+"px";
                    var y = event.pageY + 10+"px";
                    $("#addSuccess").css({"display":"inline-block","top":y,"left":x});
                    $("#addSuccess").fadeOut(2000);
                    $(event.target).parent().parent().fadeOut(2000);
                }
            }else{
                return;
            }
        },
        getSheetByYeGou:function(){
            userLocalStorage = window.localStorage
            var tempSheet = this.sheet
            if(userLocalStorage.getItem('username') != null){
                var loginname = userLocalStorage.getItem('username');
                ref.child(loginname).child("mvSheet").on("value",function(snapshot){
                    snapshot.forEach(function(snap){
                        if(tempSheet.indexOf(snap.key()) == -1){
                            tempSheet.push(snap.key());
                        }
                    });
                });
            }
            this.sheet = tempSheet
        },
        showSheet:function(event){
            if(userLocalStorage.getItem('username') != null){
                if(event.target.nodeName == 'SPAN'){
                    var $targetNode = $(event.target).parent().siblings(".addToSheet")
                }else if(event.target.nodeName == 'I'){
                    var $targetNode = $(event.target).parent().parent().siblings(".addToSheet")
                }
                if(this.sheet.length == 0){
                    $(event.target).siblings(".hasnoSheet").css('display', 'inline-block');
                }else{
                    $(event.target).siblings(".hasnoSheet").css('display', 'none');
                    $targetNode.css('display', 'block');
                }
            }else{
                $("#login").trigger('click');
            }
        },
        addNewSheet:function(event){
            var username = userLocalStorage.getItem('username');
            var sheetname = $(event.target).siblings(".newSheetInputDiv").val();

            ref.child(username).child("mvSheet").child(sheetname).push("");
            $(event.target).siblings(".addSheetSuccess").css('display', 'inline-block');
            $(event.target).parent().fadeOut(4000);
            $(event.target).parent().siblings(".hasnoSheet").fadeOut(4000);
        }
    },
    mounted:function(){
        this.getRecommendByAjax()
        this.getUserSearchHistory()
        this.getSheetByYeGou()
    },
    updated:function(){
        $(".setScorediv").css("left",$(".onefilm").width() + parseInt($(".onefilm").css('padding-right')) - 2 +'px');
        //图片和轮播图高度设置
        $(".imgdiv").height($(".imgdiv").width()*124/220);
        $(".rotation").height($(".rotation").width()*410/1500);
        $(".swiper-slide").height($(".swiper-slide").width()*410/1500);
        if($(".swiper-slide").width() < 426){
            $(".rotation").height($(".rotation").width()*410/1048);
            $(".swiper-slide").height($(".swiper-slide").width()*410/1048);
        }
    }
})
