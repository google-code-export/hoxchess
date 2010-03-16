/***************************************************************************
 *  Copyright 2009-2010 Nevo Hua  <nevo.hua@playxiangqi.com>               *
 *                                                                         * 
 *  This file is part of NevoChess.                                        *
 *                                                                         *
 *  NevoChess is free software: you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  NevoChess is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with NevoChess.  If not, see <http://www.gnu.org/licenses/>.     *
 ***************************************************************************/

#import <UIKit/UIKit.h>


// --------------------------------------
@protocol ItemInputDelegate <NSObject>
// button == nil on Cancel
- (void) handleItemInput:(NSString *)button input:(NSString*)input;
@end

// --------------------------------------
@interface ItemInputViewController : UIViewController
{
    IBOutlet UITextField* textField;
    id <ItemInputDelegate> delegate;
}

@property (nonatomic, retain) id <ItemInputDelegate> delegate;

- (IBAction)enterButtonPressed:(id)sender;
- (IBAction)cancelButtonPressed:(id)sender;

@end
