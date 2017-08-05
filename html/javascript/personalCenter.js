$(document).ready(function(){
    //用户web存储
    var userLocalStorage=window.localStorage;
    if(userLocalStorage.getItem('username') != null){
        var userStorageName = userLocalStorage.getItem('username');
        $('#login').css('display','none');
        $('#signup').css('display','none');
        $('.login-user').css('display','inline-block');
        $(".login-user").text(userStorageName);
        $("#content").removeClass('hide');
        $("#not-login").addClass('hide');
        $("#nav-username").text(userStorageName);
    }else{
        $("#content").addClass('hide');
        $("#not-login").removeClass('hide');
    }
    //搜索框提交
    $("#mainSearch").click(function(){
        $("#topsearchinputdiv").submit();
    });
    //enter键触发form表单提交搜索
    $("#searchinput").keypress(function(event) {
        if(event.keyCode =="13"){
            $("#mainSearch").trigger('click');
        }
    });
    //主体部分
    //上传头像
    $("#avatar").click(function(){
        $("#upLoadHeadImg").click();
    });
    $("#upLoadHeadImg").change(function(){
        var inputfiles = document.querySelector('input[type=file]').files;
        if(inputfiles.length > 0){
            var file = inputfiles[0];
            var reader  = new FileReader();
            reader.addEventListener("load", function () {
                $("#headImg").attr("src",reader.result);
                var username = userLocalStorage.getItem('username');
                ref.child(username).child("headimg").set({
                    url:reader.result
                });
            }, false);
            if(file){
                reader.readAsDataURL(file);
            }
        }
    });
    //新建影单
    $("#newSheet").click(function() {
        $("#newSheetDiv").css('display','block');
    });
    $("#addsheet").click(function() {
        var username = userLocalStorage.getItem('username');
        var sheetname = $("#newSheetInputDiv").val();
        ref.child(username).child("mvSheet").child(sheetname).push("");
        $("#newSheetDiv").fadeOut(2000);
    });
    $("#newSheetInputDiv").keypress(function(event) {
        if(event.keyCode =="13"){
            $("#addsheet").trigger('click');
        }
    });
    //添加好友
    $("#newfriend").click(function() {
        $("#newfriendDiv").css('display','block');
    });
    $("#addfriend").click(function() {
        var username = userLocalStorage.getItem('username');
        var friendname = $("#newfriendInputDiv").val();
        //检查好友是否存在
        if(friendname != ""){
            ref.once("value",function(snapshot) {
                var hasPerson = snapshot.hasChild(friendname);
                if(hasPerson == true){
                    ref.child(username).child("friend").child(friendname).push("");
                    $("#newfriendDiv").fadeOut(2000);
                }else{
                    $("#nopersoninfo").css('display', 'inline-block');
                    $("#nopersoninfo").fadeOut(2000);
                }
            });
        }
    });
    $("#newfriendInputDiv").keypress(function(event) {
        if(event.keyCode =="13"){
            $("#addfriend").trigger('click');
        }
    });
    $(".close-addsheet").click(function(event) {
        $("#newSheetDiv").css('display','none');
        $("#newfriendDiv").css('display','none');
        event.stopPropagation();
    });
});
