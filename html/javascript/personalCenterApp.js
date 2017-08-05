var ref = new Wilddog("https://lmagic.wilddogio.com/iqiyi/");
var app = new Vue({
    el:"#app",
    data:{
        collect:[],
        sheet:[],
        collectNum:0,
        sheetNum:0,
        friend:[],
        friendname:[],
        friendNum:0,
        logoimg:"",
    },
    methods:{
        getJsonByYeGou:function(){
            userLocalStorage = window.localStorage
            var tempCollect = this.collect
            var tempSheet = this.sheet
            var tempFriend = this.friend
            var tempfriendname = this.friendname
            var that = this
            if(userLocalStorage.getItem('username') != null){
                var loginname = userLocalStorage.getItem('username');
                ref.child(loginname).child("collect").on("value",function(snapshot){
                    snapshot.forEach(function(snap){
                        if( JSON.stringify(tempCollect).indexOf(JSON.stringify(snap.val())) == -1){
                            tempCollect.push(snap.val());
                        }
                    });
                    that.collectNum = tempCollect.length
                });
                ref.child(loginname).child("mvSheet").on("value",function(snapshot){
                    snapshot.forEach(function(snap){
                        if(tempSheet.indexOf(snap.key()) == -1){
                            tempSheet.push(snap.key());
                        }
                    });
                    that.sheetNum = tempSheet.length
                });
                ref.child(loginname).child("friend").on("value",function(snapshot){
                    snapshot.forEach(function(snap){
                        if(tempfriendname.indexOf(snap.key()) == -1){
                            var getname = snap.key();
                            tempfriendname.push(getname);
                            var geturl = "images/user.jpg";
                            ref.child(snap.key()).child("headimg").once("value",function(data) {
                                if(data.val() != null){
                                    geturl = data.val().url;
                                }
                                var obj = {
                                    name: getname,
                                    url: geturl
                                }
                                tempFriend.push(obj);
                            });
                        }
                    });
                    that.friendNum = tempfriendname.length
                });
                ref.child(loginname).once("value",function(snapshot) {
                    var hasLogoImg = snapshot.hasChild("headimg");
                    if(hasLogoImg == true){
                        that.logoimg = snapshot.child("headimg").val().url;
                    }else{
                        that.logoimg = "images/user.jpg";
                    }
                });
            }
            this.collect = tempCollect
            this.sheet = tempSheet
            this.friend = tempFriend
            this.friendname = tempfriendname
            $(".collectA").height($(".collectA").width()*124/220);
            $(".oneCollect").height($(".collectA").height()+45);
            $(".friendDiv").height($(".friendDiv").width());
        },
        animateIcon:function(event){
            $(event.target).find(".icondiv").css('bottom', "-20px");
            $(event.target).find(".collectTitle").css('color', "#77a610");
            $(event.target).find(".fa-trash").css("visibility","visible");
        },
        stopAnimateIcon:function(event){
            $(event.target).find(".icondiv").css('bottom', "-25px");
            $(event.target).find(".collectTitle").css('color', "#000");
            $(event.target).find(".fa-trash").css("visibility","hidden");
        },
        storageSheetName:function(x,event){
            userLocalStorage = window.localStorage
            userLocalStorage.setItem("sheetname", x);
        },
        getLink:function(x){
            return 'iqiyi://mobile/player?aid=' + x.aid + '&tvid=' + x.tvid + '&ftype=27&to=3&url=' + encodeURIComponent(window.location.href);
        },
        removeCollect:function(x){
            var loginname = userLocalStorage.getItem('username');
            ref.child(loginname).child("collect").child(x.aid).remove();
            this.collect.splice(this.collect.indexOf(x),1);
            this.collectNum--;
        },
        removeSheet:function(x){
            var loginname = userLocalStorage.getItem('username');
            ref.child(loginname).child("mvSheet").child(x).remove();
            this.sheet.splice(this.sheet.indexOf(x),1);
            this.sheetNum--;
        }
    },
    created:function(){
        this.getJsonByYeGou();
    }
})
