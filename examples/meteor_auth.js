if (Meteor.isClient)
{
    Meteor.startup(() => {
        // connect to C++ ddp server
        MyConnection = DDP.connect("http://localhost:9002");
        
        Tracker.autorun(function () {
            if (MyConnection.status().connected)
            {
                if (Meteor.userId())
                {
                    // send user id and token to C++ server application
                    MUDConnection.call(
                        "login",
                        Meteor.userId(),
                        Meteor._localStorage.getItem('Meteor.loginToken'),
                        function(error: any, result: any) {
                            // Authentication callback
                        }
                    );
                }
                
            }
        });
    });
    
}
else
{
    // Meteor server-side route for authentication (using Iron Router)
    // C++ application uses it to validate authentication data sent by clients
    Router.route('/checkLoginToken/:token', function () {
        
        if (!this.params.query.userId)
            this.response.end('500');
        else
        {
            // find user with same user id and token
            var user = Meteor.users.findOne({
                _id: this.params.query.userId,
                'services.resume.loginTokens.hashedToken' : Accounts._hashLoginToken(this.params.token)
            });
            if (!user)
                this.response.end('404');
            else
                this.response.end('200');
            
        }
        
    }, { where: 'server' });
}
