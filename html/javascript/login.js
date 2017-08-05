$(document).ready(function(){
     //用户背景存储
    var userLocalStorage=window.localStorage;
    if(userLocalStorage.getItem('skin') != null){
        var skinImg = userLocalStorage.getItem('skin');
        $("body").css("backgroundImage",skinImg);
    }else{
        $("body").css("backgroundImage",'url(images/back1.jpg)');
    }
    if(userLocalStorage.getItem('username') != null){
        var userStorageName = userLocalStorage.getItem('username');
        $('#login').css('display','none');
        $('#signup').css('display','none');
        $('.login-user').css('display','inline-block');
        $(".login-user").text(userStorageName);
    }
    //注册
    var ref = new Wilddog("https://lmagic.wilddogio.com/iqiyi/");
    var canSignUp = false;
    $("#signup").click(function(){
        $('#login-back').css('display','block');
        $('#signup-container').css('display','block');
    });
    $("#signup-container .close-signup").click(function(){
        $('#login-back').css('display','none');
        $('#signup-container').css('display','none');
    });
    //检查用户名是否已经注册
    $("#signupname").change(function(){
        var signname = $("#signupname").val();
        if(signname != ""){
             ref.child(signname).once('value', function(snapshot){
                if(snapshot.child("ps").val() != null){
                    $("#singup-occupy").css('visibility', 'visible');
                    canSignUp = false;
                }else{
                    $("#singup-occupy").css('visibility', 'hidden');
                    canSignUp = true;
                }
            });
        }
    });
    $(".signup-btn").click(function(){
        if(($("#signupname").val()=="")||($("#signps").val()=="")){
            alert("输入不能为空！");
        }else if(canSignUp == true){
            var signName = $("#signupname").val();
            var signPs = $("#signps").val();
            ref.child(signName).set({
                ps:signPs
            })
            $(".infoTologin").css('display','block');
        }
    });
    $("#signps").keypress(function(event) {
        if(event.keyCode =="13"){
            $(".signup-btn").trigger('click');
        }
    });
    $("#signtologin").click(function(){
        $('#signup-container').css('display','none');
        $("#login").trigger('click');
    });
    $("#loginTosign").click(function(){
        $('#login-container').css('display','none');
        $("#signup").trigger('click');
    });

    //登录
    var canlogin = false;
    $("#login").click(function(){
        $('#login-back').css('display','block');
        $('#login-container').css('display','block');
    });
    $("#login-container .close-login").click(function(){
        $('#login-back').css('display','none');
        $('#login-container').css('display','none');
    });
    //检查用户名是否正确
    $("#username").change(function(){
        var loginname = $("#username").val();
        if(loginname != ""){
            ref.child(loginname).once('value', function(snapshot){
                if(snapshot.child("ps").val() == null){
                    $("#login-error").css('visibility', 'visible');
                    canlogin = false;
                }else{
                    $("#login-error").css('visibility', 'hidden');
                    canlogin = true;
                }
            });
        }
    });
    //登陆check
    $(".login-btn").click(function(){
        if(canlogin == true){
            var loginname = $("#username").val();
            var psword = $("#psword").val();
            ref.child(loginname).once('value', function(snapshot){
                if(snapshot.child("ps").val() == psword){
                    $("#psword-error").css('visibility', 'hidden');
                    $('#login-container').css('display','none');
                    $('#login-back').css('display','none');
                    $('#login').css('display','none');
                    $('#signup').css('display','none');
                    $('.login-user').css('display','inline-block');
                    $(".login-user").text(loginname);
                    userLocalStorage.setItem("username", loginname);
                    window.location.reload();
                }else{
                    $("#psword-error").css('visibility', 'visible');
                }
            });
        }
    });
    $("#psword").keypress(function(event) {
        if(event.keyCode =="13"){
            $(".login-btn").trigger('click');
        }
    });
    $(".login-user").click(function(){
        $('#myinfo').css('display','block');
    });
    $("#myinfo .close-login").click(function(){
        $('#myinfo').css('display','none');
    });
    //退出登录
    $("#login-out").click(function() {
        $('#login').css('display','inline-block');
        $('#signup').css('display','inline-block');
        $('.login-user').css('display','none');
        $("#myinfo").css('display','none');
        userLocalStorage.removeItem("username");
        window.location.reload();
    });
    $("#centerLink").click(function() {
        $("#myinfo").css('display', 'none');
    });
});
