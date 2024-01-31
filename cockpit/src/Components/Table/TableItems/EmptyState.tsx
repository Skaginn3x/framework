import React from 'react';
import {
  Button, EmptyState, EmptyStateBody, EmptyStateFooter, EmptyStateIcon, Title,
} from '@patternfly/react-core';
import { SearchIcon } from '@patternfly/react-icons';

const emptyStateComponent = (clearAll:any) => (
  <EmptyState>
    <EmptyStateIcon icon={SearchIcon} />
    <Title size="lg" headingLevel="h4">
      No results found
    </Title>
    <EmptyStateBody>No results match the filter criteria. Clear all filters and try again.</EmptyStateBody>
    <EmptyStateFooter>
      <Button
        variant="link"
        onClick={() => clearAll()}
      >
        Clear all filters
      </Button>
    </EmptyStateFooter>
  </EmptyState>
);

export default emptyStateComponent;
