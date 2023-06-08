import React from 'react';
import { EmptyState, EmptyStateVariant, EmptyStateIcon, Title, Button } from '@patternfly/react-core';
import { ExclamationCircleIcon } from '@patternfly/react-icons';
import { useNavigate } from 'react-router';

const NotFoundPage = () => {
   const history = useNavigate();

   return (
      <EmptyState variant={EmptyStateVariant.full}>
         <EmptyStateIcon icon={ExclamationCircleIcon} />
         <Title size="lg" headingLevel="h4">
            404 Page not found
         </Title>
         <Button variant="primary" onClick={() => history(-1)}>
            Return to previous page
         </Button>
      </EmptyState>
   );
};

export default NotFoundPage;
