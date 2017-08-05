$(document).ready(function(){
    $(window).resize(function() {
      //图片和轮播图高度设置
        $(".imgdiv").height($(".imgdiv").width()*124/220);
        $(".rotation").height($(".rotation").width()*410/1500);
        $(".swiper-slide").height($(".swiper-slide").width()*410/1500);
        if($(".swiper-slide").width() < 426){
           /* $("#slide1-img").attr('src','images/rotation1-1.jpg');*/
            $(".rotation").height($(".rotation").width()*410/1048);
            $(".swiper-slide").height($(".swiper-slide").width()*410/1048);
        }
    });
    //用户存储
    var userLocalStorage=window.localStorage;
    //换肤效果
    $("#skinning").mouseover(function(){
        $("#skinningSelect").removeClass('hide');
    });
    $("#skinningSelect").mouseleave(function(){
        $("#skinningSelect").addClass('hide');
    });
    $("#skinningSelect").click(function(event){
        var buttonId = event.target.id;
        switch(true){
            case buttonId=="fresh":
                $("body").css("backgroundImage",'url(images/back13.jpg)');
                userLocalStorage.setItem("skin", 'url(images/back13.jpg)');
                break;
            case buttonId=="sport":
                $("body").css("backgroundImage",'url(images/back16.jpg)');
                userLocalStorage.setItem("skin", 'url(images/back16.jpg)');
                break;
            case buttonId=="cute":
                $("body").css("backgroundImage",'url(images/back1.jpg)');
                userLocalStorage.setItem("skin", 'url(images/back1.jpg)');
                break;
            case buttonId=="cool":
                $("body").css("backgroundImage",'url(images/back2.jpg)');
                userLocalStorage.setItem("skin", 'url(images/back2.jpg)');
                break;
            case buttonId=="closeSkin":
                $("body").css("backgroundImage","none");
                userLocalStorage.setItem("skin", "none");
                break;
            default:
                $("body").css("backgroundImage","none");
        }
    });
    //搜索框提交
    $("#mainSearch").click(function(){
        if(JSON.parse(userLocalStorage.getItem('searchHistory')) == null){
            userLocalStorage.setItem('searchHistory',JSON.stringify([]));
        };
        var searchhistory = JSON.parse(userLocalStorage.getItem('searchHistory'));
        //设置搜索历史的存储条数最多为5
        if(searchhistory.length > 4){
                searchhistory = searchhistory.slice(0,4)
        };
        var searchText = $("#searchinput").val();
        if(searchhistory.indexOf(searchText) == -1){
            searchhistory.unshift($("#searchinput").val());
        }
        userLocalStorage.setItem('searchHistory',JSON.stringify(searchhistory));
        $("#topsearchinputdiv").submit();
    });
    //enter键触发form表单提交搜索
    $("#searchinput").keypress(function(event) {
        if(event.keyCode =="13"){
            $("#mainSearch").trigger('click');
        }
    });
    //星星评分
    $(".setScorediv").css("left",$(".onefilm").width() + parseInt($(".onefilm").css('padding-right')) - 2 +'px');
    $(".container-fluid").mouseover(function(event){
        if(event.target.nodeName == 'IMG'){
           $(event.target).parent().parent().siblings('.setScorediv').removeClass('hide');
        }
        $('.star').barrating({
            theme: 'fontawesome-stars-o',
            showSelectedRating: true,
        });
    });
    //搜索框推荐
    $("#searchinput").focus(function() {
        clearTimeout(timer);
        $('.searchRecommend').removeClass('hide');
    });
    var timer = null;
    $("#searchinput").blur(function(){
        timer = setTimeout(function(){
                    $(".searchRecommend").addClass('hide');
                },500);
    });
    $(".searchRecommend").mouseover(function(){
        clearTimeout(timer);
    });
    //自定义皮肤文件上传
    $("#selfDefinition").click(function(){
        $("#upLoadImg").click();
    });
    $("#upLoadImg").change(function(){
        var inputfiles = document.querySelector('input[type=file]').files;
        if(inputfiles.length > 0) {
            var file = inputfiles[0];
            var reader  = new FileReader();
            reader.addEventListener("load", function () {
                $("body").css("backgroundImage",'url('+reader.result+')');
                userLocalStorage.setItem("skin", 'url('+reader.result+')');
                }, false);
            if(file){
                reader.readAsDataURL(file);
            }
        }
    });
    //刷新按钮
    $(".reloadbutton").click(function(){
        window.location.reload();
    });
    //导航栏
    $("#checkoutRightChannel").click(function(){
        var wid = $("#checkoutRightChannel").width();
        $(".realChannel").each(function(){
            wid = wid + $(this).width();
          });
        $(".realChannel").animate({right:wid+"px"}, 500);
        $("#checkoutRightChannel").animate({right:wid+"px"}, 500);
        $("#checkoutLeftChannel").animate({right:wid+"px"}, 500);
        $(".backupchannel").animate({right:wid+"px"}, 500);
        $(".realChannel").addClass('notvisible');
        $("#checkoutRightChannel").addClass('notvisible');
        $(".backupchannel").removeClass('notvisible');
        $("#checkoutLeftChannel").removeClass('notvisible');
    });
    $("#checkoutLeftChannel").click(function(){
        var wid = 0;
        $(".realChannel").animate({right:wid+"px"}, 500);
        $("#checkoutRightChannel").animate({right:wid+"px"}, 500);
        $("#checkoutLeftChannel").animate({right:wid+"px"}, 500);
        $(".backupchannel").animate({right:wid+"px"}, 500);
        $(".backupchannel").addClass('notvisible');
        $("#checkoutLeftChannel").addClass('notvisible');
        $(".realChannel").removeClass('notvisible');
        $("#checkoutRightChannel").removeClass('notvisible');
    });
    //频道提示语相关
    $(".backupchannel").click(function() {
        $("#content").addClass('hide');
        $("#prompt").removeClass('hide');
    });
    $(".realChannel").click(function() {
        $("#content").removeClass('hide');
        $("#prompt").addClass('hide');
    });
    /*收藏与影单*/
    $(".addAsheet").click(function(event){
        $(event.target).parent().siblings('.newSheetDiv').css('display', 'block');
    });
    $(".newSheetInputDiv").keypress(function(event) {
        if(event.keyCode =="13"){
           $(event.target).siblings(".addsheet").trigger('click');
        }
    });
    $(".close-addsheet").click(function(event) {
        $(event.target).parent().css('display','none');
        $(event.target).parent().siblings(".hasnoSheet").css('display','none');
        event.stopPropagation();
    });
    $(".newSheetButton").click(function(event) {
        $(event.target).parent().parent().siblings(".userplus").children('.newSheetDiv').css('display', 'block');
    });
});
